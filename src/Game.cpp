#include "Game.hpp"

#include "GameConstants.hpp"

#include <algorithm>
#include <filesystem>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>

#ifndef BUG_TOSS_ASSET_DIR
#error "BUG_TOSS_ASSET_DIR must be provided by the build system"
#endif

namespace bug_toss
{
namespace
{
std::string assetPath(const char* filename)
{
    return (std::filesystem::path(BUG_TOSS_ASSET_DIR) / filename).string();
}

sf::Texture loadPumbaTexture()
{
    sf::Image image;
    const std::string path = assetPath("images/Pumbaa.png");
    if (!image.loadFromFile(path))
    {
        throw std::runtime_error("Failed to load Pumbaa texture: " + path);
    }

    image.createMaskFromColor(sf::Color(255, 0, 255));
    return sf::Texture(image);
}

std::vector<sf::Vector2f> makeDropperPath()
{
    std::vector<sf::Vector2f> path;
    path.reserve(constants::DROPPER_PATH.size());
    for (const constants::Point point : constants::DROPPER_PATH)
    {
        path.emplace_back(point.x, point.y);
    }
    return path;
}
}

Game::Game()
    : window_(sf::VideoMode({constants::INTERNAL_WIDTH * constants::INITIAL_WINDOW_SCALE,
                             constants::INTERNAL_HEIGHT * constants::INITIAL_WINDOW_SCALE}),
              "Bug Toss"),
      view_(sf::FloatRect({0.f, 0.f},
                          {static_cast<float>(constants::INTERNAL_WIDTH),
                           static_cast<float>(constants::INTERNAL_HEIGHT)})),
      backgroundTexture_(assetPath("images/bug_toss_background.png")),
      background_({static_cast<float>(constants::INTERNAL_WIDTH),
                   static_cast<float>(constants::INTERNAL_HEIGHT)}),
      startBackgroundTexture_(assetPath("images/start_background.png")),
      startBackground_({static_cast<float>(constants::INTERNAL_WIDTH),
                        static_cast<float>(constants::INTERNAL_HEIGHT)}),
      menuFont_(assetPath("fonts/UncialAntiqua-Regular.ttf")),
      playText_(menuFont_, "Play", constants::MENU_TEXT_SIZE),
      scoreText_(menuFont_, constants::MENU_TEXT_SIZE, score_, constants::TEXT_ANIMATION_DURATION),
      bugTexture_(assetPath("images/Bugs.png")),
      timonTexture_(assetPath("images/Timon.png")),
      pumbaTexture_(loadPumbaTexture()),
      randomEngine_(std::random_device{}()),
      bugPick_(0, static_cast<int>(constants::BUG_FRAMES.size()) - 1),
      gravityPick_(0, static_cast<int>(constants::BUG_GRAVITIES.size()) - 1),
      spawnDelayPick_(constants::MINIMUM_SPAWN_DELAY,
                      constants::MAXIMUM_SPAWN_DELAY),
      player_(pumbaTexture_, constants::PLAYER_SPEED, {constants::PLAYER_START_X, constants::PLAYER_START_Y}),
      dropper_(timonTexture_, constants::DROPPER_SPEED, randomEngine_, makeDropperPath()),
      difficultyThreshold_(constants::DIFFICULTY_THRESHOLD)
{
    window_.setMinimumSize(
        sf::Vector2u{constants::INTERNAL_WIDTH, constants::INTERNAL_HEIGHT});

    background_.setTexture(&backgroundTexture_);
    startBackground_.setTexture(&startBackgroundTexture_);
    backgroundTexture_.setSmooth(false);
    startBackgroundTexture_.setSmooth(false);
    bugTexture_.setSmooth(false);
    timonTexture_.setSmooth(false);
    pumbaTexture_.setSmooth(false);

    playText_.setFillColor(sf::Color(230, 227, 5));
    playText_.setCharacterSize(24);
    centerText(playText_, {static_cast<float>(constants::INTERNAL_WIDTH) / 2.f,
                           static_cast<float>(constants::INTERNAL_HEIGHT) / 2.f});
    centerText(scoreText_.text(), {static_cast<float>(constants::INTERNAL_WIDTH) / 2.f,
                            static_cast<float>(constants::INTERNAL_HEIGHT) / 2.f});

    updateViewport(window_.getSize());
}

void Game::run()
{
    sf::Clock clock;
    while (window_.isOpen())
    {
        processEvents();
        if (!window_.isOpen())
        {
            break;
        }

        const sf::Time dt = clock.restart();
        update(dt);
        render();
    }
}

void Game::processEvents()
{
    while (const std::optional event = window_.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window_.close();
        }

        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::Escape)
            {
                window_.close();
            }

            if (state_ != State::Play && keyPressed->code == sf::Keyboard::Key::Enter)
            {
                startGame();
            }

            if (state_ == State::Play && burpAttempts_ > 0 &&
                keyPressed->code == sf::Keyboard::Key::Space)
            {
                burpAttempts_--;
                bugs_.clear();
                audioManager_.playSound(AudioManager::SoundEffect::Burp);
                player_.startBurping();
            }
        }

        if (const auto* mouseButton = event->getIf<sf::Event::MouseButtonPressed>())
        {
            if (mouseButton->button == sf::Mouse::Button::Left)
            {
                handleMenuActivation(
                    window_.mapPixelToCoords(mouseButton->position, view_));
            }
        }

        if (const auto* resized = event->getIf<sf::Event::Resized>())
        {
            updateViewport(resized->size);
        }
    }
}

void Game::update(sf::Time dt)
{
    if (state_ == State::Start)
    {
        return;
    }
    if (state_ == State::End)
    {
        scoreText_.update(dt);
        return;
    }

    float playerDirection = 0.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
    {
        playerDirection -= 1.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
    {
        playerDirection += 1.f;
    }

    player_.setMovementDirection(playerDirection);
    player_.update(dt);
    dropper_.update(dt);

    bool isBurping = player_.state() == Player::State::Burp;

    for (Bug& bug : bugs_)
    {
        bug.update(dt);
    }

    std::erase_if(bugs_, [this](const Bug& bug)
    {
        if (bug.position().y >= constants::BUG_BOTTOM)
        {
            if (!bug.isDeadly()) endGame();
            return true;
        }

        if (bug.bounds().findIntersection(player_.bounds()))
        {
            player_.startEating();
            if (bug.isDeadly())
            {
                endGame();
            }
            else
            {
                audioManager_.playSound(AudioManager::SoundEffect::Eat);
                ++score_;
                if (score_ >= difficultyThreshold_)
                {
                    difficulty_ = Difficulty::Hard;
                }
                if (score_ >= 99)
                {
                    endGame();
                }
            }
            return true;
        }

        return false;
    });

    spawnTimer_ -= dt.asSeconds();
    if (!isBurping && spawnTimer_ <= 0.f && bugs_.size() < constants::MAX_BUGS)
    {
        spawnBug();
        spawnTimer_ = nextSpawnDelay();
    }
}

void Game::render()
{
    window_.setView(view_);
    if (state_ == State::Start)
    {
        window_.clear();
        window_.draw(startBackground_);
        window_.draw(playText_);
    }
    else if (state_ == State::Play)
    {
        window_.clear();
        window_.draw(background_);
        for (const Bug& bug : bugs_)
        {
            bug.draw(window_);
        }
        player_.draw(window_);
        dropper_.draw(window_);
    }
    else
    {
        window_.clear(sf::Color::Black);
        scoreText_.draw(window_);
    }
    window_.display();
}

void Game::startGame()
{
    if (state_ != State::Start && state_ != State::End)
    {
        return;
    }

    score_ = 0;
    bugs_.clear();
    player_.reset();
    dropper_.reset();
    scoreText_.reset();
    burpAttempts_ = 1;
    difficulty_ = Difficulty::Easy;
    spawnTimer_ = nextSpawnDelay();

    audioManager_.playMusic();
    state_ = State::Play;
}

void Game::spawnBug()
{
    const auto deadlyCount = std::count_if(bugs_.begin(), bugs_.end(),
    [](const Bug& b) { return b.isDeadly(); });

    const bool canSpawnDeadly = bugs_.size() <= 1;
    const bool canSpawnSafe   = deadlyCount <= 1;

    if (!canSpawnDeadly && !canSpawnSafe)
    {
        return;
    }

    float gravity = constants::BUG_GRAVITIES[gravityPick_(randomEngine_)];
    if (difficulty_ == Difficulty::Hard)
    {
        gravity *= constants::HARD_GRAVITY_MULTIPLIER;
    }
    int bugIndex;

    if (canSpawnDeadly && canSpawnSafe)
    {
        bugIndex = bugPick_(randomEngine_);
        bugs_.emplace_back(bugTexture_, dropper_.position(), gravity, bugIndex);
    }
    else if (!canSpawnSafe)
    {
        bugIndex = static_cast<int>(constants::BUG_FRAMES.size()) - 1;
        bugs_.emplace_back(bugTexture_, dropper_.position(), gravity, bugIndex);
    }
    else 
    {
        do {
            bugIndex = bugPick_(randomEngine_);
        } while(constants::BUG_DEADLINESS[bugIndex]);

        bugs_.emplace_back(bugTexture_, dropper_.position(), gravity, bugIndex);
    }
}

float Game::nextSpawnDelay()
{
    const float difficultyMultiplier =
        difficulty_ == Difficulty::Hard ? constants::HARD_SPAWN_DELAY_MULTIPLIER : 1.f;
    return spawnDelayPick_(randomEngine_) * difficultyMultiplier;
}

void Game::endGame()
{
    state_ = State::End;
    scoreText_.setTotalScore(score_);
    centerText(scoreText_.text(), {static_cast<float>(constants::INTERNAL_WIDTH) / 2.f,
                            static_cast<float>(constants::INTERNAL_HEIGHT) / 2.f});
    if (score_ >= difficultyThreshold_) audioManager_.playSound(AudioManager::SoundEffect::Victory); 
    audioManager_.stopMusic();
}

void Game::centerText(sf::Text& text, sf::Vector2f position) const
{
    const sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({bounds.position.x + bounds.size.x / 2.f,
                    bounds.position.y + bounds.size.y / 2.f});
    text.setPosition(position);
}

void Game::handleMenuActivation(sf::Vector2f position)
{
    if (state_ == State::Start && playText_.getGlobalBounds().contains(position))
    {
        startGame();
    }
    else if (state_ == State::End && scoreText_.text().getGlobalBounds().contains(position))
    {
        startGame();
    }
}

void Game::updateViewport(sf::Vector2u windowSize)
{
    if (windowSize.x == 0 || windowSize.y == 0)
    {
        return;
    }

    const unsigned int scale = std::max(
        1U, std::min(windowSize.x / constants::INTERNAL_WIDTH,
                     windowSize.y / constants::INTERNAL_HEIGHT));
    const float viewportWidth =
        static_cast<float>(constants::INTERNAL_WIDTH * scale) / static_cast<float>(windowSize.x);
    const float viewportHeight =
        static_cast<float>(constants::INTERNAL_HEIGHT * scale) / static_cast<float>(windowSize.y);
    view_.setViewport(sf::FloatRect({(1.f - viewportWidth) / 2.f,
                                     (1.f - viewportHeight) / 2.f},
                                    {viewportWidth, viewportHeight}));
    window_.setView(view_);
}
}
