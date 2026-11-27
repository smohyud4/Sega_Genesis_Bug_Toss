#include "Game.hpp"

#include "GameConstants.hpp"

#include <algorithm>
#include <filesystem>
#include <optional>
#include <random>
#include <cmath>
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
      dropper_(timonTexture_, constants::DROPPER_SPEED, randomEngine_, makeDropperPath())
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
    audioManager_.playMusic(AudioManager::MusicTrack::Start);

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
    updateJoystick();

    while (const std::optional event = window_.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            window_.close();
        }

        bool startPressed = false;
        bool burpPressed = false;
        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::Escape)
            {
                window_.close();
            }

            startPressed = keyPressed->code == sf::Keyboard::Key::Enter;
            burpPressed = keyPressed->code == sf::Keyboard::Key::Space;
        }

        if (const auto* joystickButton = event->getIf<sf::Event::JoystickButtonPressed>())
        {
            if (joystickId_ && joystickButton->joystickId == *joystickId_ &&
                joystickButton->button == 0)
            {
                startPressed = true;
                burpPressed = true;
            }
        }

        if (burpPressed && state_ == State::Play && player_.burps() > 0)
        {
            bugs_.clear();
            audioManager_.playSound(AudioManager::SoundEffect::Burp);
            player_.startBurping();
        }

        if (state_ != State::Play && startPressed && scoreText_.currScore() == score_)
        {
            startGame();
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
    playerDirection = std::clamp(playerDirection + joystickDirection(), -1.f, 1.f);

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
                updateDifficulty();
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
        if (spawnBug())
        {
            spawnTimer_ = nextSpawnDelay();
        }
    }
}

void Game::updateJoystick()
{
    sf::Joystick::update();
    joystickId_.reset();

    std::optional<unsigned int> connectedJoystick;
    for (unsigned int joystick = 0; joystick < sf::Joystick::Count; ++joystick)
    {
        if (!sf::Joystick::isConnected(joystick))
        {
            continue;
        }

        if (connectedJoystick)
        {
            return;
        }
        connectedJoystick = joystick;
    }

    if (connectedJoystick && sf::Joystick::getButtonCount(*connectedJoystick) > 0 &&
        sf::Joystick::hasAxis(*connectedJoystick, sf::Joystick::Axis::X))
    {
        joystickId_ = connectedJoystick;
    }
}

float Game::joystickDirection() const
{
    if (!joystickId_)
    {
        return 0.f;
    }

    const float axisPosition =
        sf::Joystick::getAxisPosition(*joystickId_, sf::Joystick::Axis::X);
    const float absolutePosition = std::abs(axisPosition);
    if (absolutePosition <= constants::JOYSTICK_DEADZONE)
    {
        return 0.f;
    }

    return axisPosition > 0.f ? 1.f : -1.f;
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
    difficulty_ = Difficulty::Easy;
    spawnTimer_ = nextSpawnDelay();

    audioManager_.stopMusic(AudioManager::MusicTrack::Start);
    audioManager_.playMusic(AudioManager::MusicTrack::Playing);
    state_ = State::Play;
}

bool Game::spawnBug()
{
    const auto deadlyCount = std::count_if(bugs_.begin(), bugs_.end(),
    [](const Bug& b) { return b.isDeadly(); });

    const bool canSpawnDeadly = bugs_.size() <= 1;
    const bool canSpawnSafe   = deadlyCount <= 1;

    if (!canSpawnDeadly && !canSpawnSafe)
    {
        return false;
    }

    float gravity = constants::BUG_GRAVITIES[gravityPick_(randomEngine_)];
    const auto& scale = constants::DIFFICULTY_SCALES[static_cast<std::size_t>(difficulty_)];
    gravity *= scale.gravityMultiplier;
    int bugIndex;

    if (canSpawnDeadly && canSpawnSafe)
    {
        bugIndex = bugPick_(randomEngine_);
    }
    else if (!canSpawnSafe)
    {
        bugIndex = static_cast<int>(constants::BUG_FRAMES.size()) - 1;
    }
    else 
    {
        do {
            bugIndex = bugPick_(randomEngine_);
        } while(constants::BUG_DEADLINESS[bugIndex]);

    }

    Bug candidate(bugTexture_, dropper_.position(), gravity, bugIndex);

    if (!isFarEnough(candidate))
    {
        return false;
    }
    
    if (player_.burps() <= 0 && !isPossibleToReach(candidate))
    {
        return false;
    }

    bugs_.emplace_back(bugTexture_, dropper_.position(), gravity, bugIndex);
    return true;
}

bool Game::isPossibleToReach(const Bug& candidate) const
{
    struct LandingBug
    {
        float landingTime;
        float horizontalPosition;
        float width;
    };

    std::vector<LandingBug> landingBugs;
    landingBugs.reserve(bugs_.size() + 1);

    for (const Bug& bug : bugs_)
    {
        
        if (!bug.isDeadly())
        {
            const sf::FloatRect bounds = bug.bounds();
            landingBugs.push_back({
                bug.timeUntilPosition(constants::BUG_BOTTOM),
                bounds.position.x + bounds.size.x / 2.f,
                bounds.size.x
            });
        }
    }

    if (!candidate.isDeadly())
    {
        const sf::FloatRect candidateBounds = candidate.bounds();
        landingBugs.push_back({
            candidate.timeUntilPosition(constants::BUG_BOTTOM),
            candidateBounds.position.x + candidateBounds.size.x / 2.f,
            candidateBounds.size.x
        });
    }

    std::sort(landingBugs.begin(), landingBugs.end(),
              [](const LandingBug& first, const LandingBug& second)
              {
                  return first.landingTime < second.landingTime;
              });

    if (landingBugs.size() < 2)
    {
        return true;
    }

    const float playerWidth = player_.bounds().size.x;
    const float playerSpeed =
        player_.movementSpeed() * constants::FAIRNESS_PLAYER_SPEED_FACTOR;

    for (std::size_t index = 1; index < landingBugs.size(); ++index)
    {
        const LandingBug& previous = landingBugs[index - 1];
        const LandingBug& current = landingBugs[index];

        const float horizontalDistance = std::abs(current.horizontalPosition - previous.horizontalPosition);
        const float averageBugHalfWidth = (previous.width + current.width) / 4.f;
        const float catchDistance =
            std::max(0.f,
                     horizontalDistance - playerWidth / 2.f -
                     averageBugHalfWidth +
                     constants::FAIRNESS_OVERLAP_TOLERANCE);

        const float landingGap = std::max(0.f, current.landingTime - previous.landingTime);

        if (catchDistance > playerSpeed * landingGap)
        {
            return false;
        }
    }

    return true;
}

bool Game::isFarEnough(const Bug& candidate) const
{
    const float candidateHorizontalPosition = candidate.bounds().position.x + candidate.bounds().size.x / 2.f;
    return std::all_of(bugs_.begin(), bugs_.end(),
        [&](const Bug& bug)
        {
            const sf::FloatRect bounds = bug.bounds();
            const float centerX = bounds.position.x + bounds.size.x / 2.f;
            return std::abs(candidateHorizontalPosition - centerX)
                >= constants::MIN_BUG_DISTANCE;
        });
}

float Game::nextSpawnDelay()
{
    const auto& scale = constants::DIFFICULTY_SCALES[
        static_cast<std::size_t>(difficulty_)];
    return spawnDelayPick_(randomEngine_) * scale.spawnDelayMultiplier;
}

void Game::updateDifficulty()
{
    if (score_ >= constants::HARD_DIFFICULTY_THRESHOLD)
    {
        difficulty_ = Difficulty::Hard;
    }
    else if (score_ >= constants::MEDIUM_DIFFICULTY_THRESHOLD)
    {
        difficulty_ = Difficulty::Medium;
    }
    else
    {
        difficulty_ = Difficulty::Easy;
    }
}

void Game::endGame()
{
    state_ = State::End;
    scoreText_.setTotalScore(score_);
    centerText(scoreText_.text(), {static_cast<float>(constants::INTERNAL_WIDTH) / 2.f,
                            static_cast<float>(constants::INTERNAL_HEIGHT) / 2.f});
    if (score_ >= constants::HARD_DIFFICULTY_THRESHOLD)
    {
        audioManager_.playSound(AudioManager::SoundEffect::Victory);
    }
    audioManager_.stopMusic(AudioManager::MusicTrack::Playing);
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
