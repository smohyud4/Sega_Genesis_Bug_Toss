#include "Player.hpp"

#include "GameConstants.hpp"
#include <iostream>

namespace bug_toss
{
namespace
{
sf::IntRect toIntRect(const constants::Frame& frame)
{
    return sf::IntRect({frame.x, frame.y}, {frame.width, frame.height});
}
}

Player::Player(const sf::Texture& texture, float speed, sf::Vector2f startPosition)
    : animations_{{toIntRect(constants::PLAYER_IDLE_FRAME),
                   toIntRect(constants::PLAYER_RUN_FRAME),
                   toIntRect(constants::PLAYER_EAT_FRAME),
                   toIntRect(constants::PLAYER_BURP_FRAME)}},
    startPosition_(startPosition),
      sprite_(texture),
      speed_(speed)
{
    sprite_.setPosition(startPosition);
    applyFrame();
}

void Player::setMovementDirection(float direction)
{
    movementDirection_ = direction;
}

void Player::update(sf::Time dt)
{
    const float deltaSeconds = dt.asSeconds();

    if (state_ == State::Burp)
    {
        burpingTime_ += deltaSeconds;
        if (burpingTime_ >= constants::PLAYER_BURPING_DURATION)
        {
            burpingTime_ = 0.f;
        }
        else
        {
            applyFrame();
            return;
        }
    }

    if (state_ == State::Eat)
    {
        eatingTime_ += deltaSeconds;
    }
    if (eatingTime_ >= constants::PLAYER_EATING_DURATION)
    {
        eatingTime_ = 0.f;
    }
    if (eatingTime_ == 0.f)
    {
        if (movementDirection_ == 0.f)
        {
            state_ = State::Idle;
        }
        else
        {
            state_ = State::Run;
            facing_ = movementDirection_;
        }
    }

    sprite_.move({movementDirection_ * speed_ * deltaSeconds, 0.f});
    applyFrame();
    clampToWorld();
}

void Player::applyFrame()
{
    const sf::IntRect& frame = animations_[static_cast<std::size_t>(state_)];
    sprite_.setTextureRect(frame);
    sprite_.setOrigin({frame.size.x / 2.f, static_cast<float>(frame.size.y)});
    sprite_.setScale({constants::PIXEL_SCALE * facing_, constants::PIXEL_SCALE});
}

void Player::clampToWorld()
{
    const sf::FloatRect bounds = sprite_.getGlobalBounds();
    if (bounds.position.x < 0.f)
    {
        sprite_.move({-bounds.position.x, 0.f});
    }
    else if (bounds.position.x + bounds.size.x > static_cast<float>(constants::INTERNAL_WIDTH))
    {
        sprite_.move({static_cast<float>(constants::INTERNAL_WIDTH) -
                          (bounds.position.x + bounds.size.x),
                      0.f});
    }
}

sf::FloatRect Player::bounds() const
{
    return sprite_.getGlobalBounds();
}

float Player::movementSpeed() const
{
    return speed_;
}

void Player::draw(sf::RenderTarget& target) const
{
    target.draw(sprite_);
}

void Player::startEating()
{
    if (state_ != State::Eat)
    {
        state_ = State::Eat;
    }
}

void Player::startBurping()
{
    if (state_ != State::Burp)
    {
        state_ = State::Burp;
        burpingTime_ = 0.f;
    }
}

void Player::reset()
{
    state_ = State::Idle;
    movementDirection_ = 0.f;
    facing_ = 1.f;
    eatingTime_ = 0.f;
    burpingTime_ = 0.f;
    sprite_.setPosition(startPosition_);
    applyFrame();
}

Player::State Player::state() const
{
    return state_;
}
}
