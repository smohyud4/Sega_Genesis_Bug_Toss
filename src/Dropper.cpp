#include "Dropper.hpp"

#include "GameConstants.hpp"

#include <cmath>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace bug_toss
{
Dropper::Dropper(const sf::Texture& texture, float speed, std::mt19937& randomEngine,
                 std::vector<sf::Vector2f> path)
    : path_(std::move(path)),
      sprite_(texture),
      speed_(speed),
      randomEngine_(randomEngine),
      turnDistance_(constants::DROPPER_STEP_DISTANCES[1])
{
    if (path_.size() < 2)
    {
        throw std::invalid_argument("Dropper path must contain at least two points");
    }

    runningAnimation_.frames = {
        sf::IntRect({13, 57}, {59, 37}),
        sf::IntRect({79, 57}, {55, 37}),
        sf::IntRect({140, 58}, {50, 36}),
        sf::IntRect({198, 58}, {53, 36}),
        sf::IntRect({263, 57}, {55, 37}),
        sf::IntRect({15, 104}, {52, 37}),
        sf::IntRect({70, 105}, {51, 36}),
        sf::IntRect({121, 105}, {54, 36})
    };
    runningAnimation_.duration = constants::DROPPER_RUNNING_DURATION;
    runningAnimation_.loop = true;

    sprite_.setPosition(path_.front());
    applyFrame();
}

void Dropper::update(sf::Time dt)
{
    const float deltaSeconds = dt.asSeconds();

    updateFrame(deltaSeconds);
    move(deltaSeconds);
    applyFrame();
}

void Dropper::move(float dt)
{
    const sf::Vector2f toTarget = path_[static_cast<std::size_t>(targetIndex_)] - sprite_.getPosition();
    const float distance = std::hypot(toTarget.x, toTarget.y);
    const float step = speed_ * dt;

    if (distance <= step)
    {
        sprite_.setPosition(path_[static_cast<std::size_t>(targetIndex_)]);

        int nextIndex = targetIndex_ + static_cast<int>(direction_);
        if (nextIndex < 0 || nextIndex >= static_cast<int>(path_.size()))
        {
            direction_ = -direction_;
            nextIndex = targetIndex_ + static_cast<int>(direction_);
        }
        targetIndex_ = nextIndex;
    }
    else
    {
        sprite_.move(toTarget / distance * step);
        distanceTraveled_ += step;

        if (distanceTraveled_ >= turnDistance_)
        {
            targetIndex_ -= static_cast<int>(direction_);
            direction_ = -direction_;
            distanceTraveled_ = 0.f;

            std::uniform_int_distribution<std::size_t> stepPick(
                0, std::size(constants::DROPPER_STEP_DISTANCES) - 1);
            turnDistance_ = constants::DROPPER_STEP_DISTANCES[stepPick(randomEngine_)];
        }
    }
}

void Dropper::updateFrame(float dt)
{
    if (runningAnimation_.time > runningAnimation_.duration) return;

    int totalFrames = runningAnimation_.frames.size();
    const float frameDuration = runningAnimation_.duration / totalFrames;

    runningAnimation_.time += dt;
    if (runningAnimation_.time > frameDuration * (runningAnimation_.frame + 1))
    {
        runningAnimation_.frame = (runningAnimation_.frame + 1) % totalFrames;
        if (runningAnimation_.frame == 0 && runningAnimation_.loop) runningAnimation_.time = 0;
    }
}

void Dropper::applyFrame()
{
    const sf::IntRect& frame = runningAnimation_.frames[runningAnimation_.frame];
    sprite_.setTextureRect(frame);
    sprite_.setOrigin({constants::DROPPER_FRAME.width / 2.f, static_cast<float>(constants::DROPPER_FRAME.height)});
    sprite_.setScale({constants::PIXEL_SCALE * direction_, constants::PIXEL_SCALE});
}

sf::Vector2f Dropper::position() const
{
    return {sprite_.getPosition().x, sprite_.getPosition().y - sprite_.getGlobalBounds().size.y / 2.f};
}

void Dropper::draw(sf::RenderTarget& target) const
{
    target.draw(sprite_);
}

void Dropper::reset()
{
    targetIndex_ = 1;
    direction_ = 1.f;
    distanceTraveled_ = 0.f;
    runningAnimation_.frame = 0;
    runningAnimation_.time = 0.f;
    turnDistance_ = constants::DROPPER_STEP_DISTANCES[1];
    sprite_.setPosition(path_.front());
    sprite_.setOrigin({constants::DROPPER_FRAME.width / 2.f,
                       static_cast<float>(constants::DROPPER_FRAME.height)});
    sprite_.setScale({constants::PIXEL_SCALE, constants::PIXEL_SCALE});
}
}
