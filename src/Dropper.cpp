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

    sprite_.setPosition(path_.front());
    sprite_.setTextureRect(sf::IntRect(
        {constants::DROPPER_FRAME.x, constants::DROPPER_FRAME.y},
        {constants::DROPPER_FRAME.width, constants::DROPPER_FRAME.height}));
}

void Dropper::update(sf::Time dt)
{
    move(dt.asSeconds());
    sprite_.setOrigin({constants::DROPPER_FRAME.width / 2.f,
                       static_cast<float>(constants::DROPPER_FRAME.height)});
    sprite_.setScale({constants::PIXEL_SCALE * direction_, constants::PIXEL_SCALE});
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
    turnDistance_ = constants::DROPPER_STEP_DISTANCES[1];
    sprite_.setPosition(path_.front());
    sprite_.setOrigin({constants::DROPPER_FRAME.width / 2.f,
                       static_cast<float>(constants::DROPPER_FRAME.height)});
    sprite_.setScale({constants::PIXEL_SCALE, constants::PIXEL_SCALE});
}
}
