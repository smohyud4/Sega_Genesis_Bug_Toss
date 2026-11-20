#include "Bug.hpp"

#include "GameConstants.hpp"

namespace bug_toss
{
Bug::Bug(const sf::Texture& texture, sf::Vector2f startPosition, float gravity, int frameIndex)
    : sprite_(texture, sf::IntRect(
          {constants::BUG_FRAMES[static_cast<std::size_t>(frameIndex)].x,
           constants::BUG_FRAMES[static_cast<std::size_t>(frameIndex)].y},
          {constants::BUG_FRAMES[static_cast<std::size_t>(frameIndex)].width,
           constants::BUG_FRAMES[static_cast<std::size_t>(frameIndex)].height})),
      gravity_(gravity),
      isDeadly_(constants::BUG_DEADLINESS[static_cast<std::size_t>(frameIndex)])
{
    sprite_.setPosition(startPosition);
    sprite_.setScale({constants::PIXEL_SCALE, constants::PIXEL_SCALE});
}

void Bug::update(sf::Time dt)
{
    speed_ += gravity_ * dt.asSeconds();
    sprite_.move({0.f, speed_ * dt.asSeconds()});
}

void Bug::draw(sf::RenderTarget& target) const
{
    target.draw(sprite_);
}

sf::Vector2f Bug::position() const
{
    return sprite_.getPosition();
}

sf::FloatRect Bug::bounds() const
{
    return sprite_.getGlobalBounds();
}

bool Bug::isDeadly() const
{
    return isDeadly_;
}
}
