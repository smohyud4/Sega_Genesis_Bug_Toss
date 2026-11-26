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
    : startPosition_(startPosition),
      sprite_(texture),
      speed_(speed)
{
    animations_[static_cast<std::size_t>(State::Run)] = {{
        sf::IntRect({ 12, 92}, {60, 42}),
        sf::IntRect({ 80, 90}, {58, 43}),
        sf::IntRect({146, 89}, {58, 44}),
        sf::IntRect({209, 89}, {55, 44}),
        sf::IntRect({271, 90}, {49, 43}),
        sf::IntRect({324, 91}, {51, 42}),
        sf::IntRect({380, 94}, {54, 39}),
        sf::IntRect({439, 94}, {67, 39}),
        sf::IntRect({510, 98}, {78, 35}),
        sf::IntRect({595, 94}, {66, 39}),
    }, constants::PLAYER_RUNNING_DURATION, 0, 0, true};

    animations_[static_cast<std::size_t>(State::Idle)] = {{
        toIntRect(constants::PLAYER_IDLE_FRAME),
    }, constants::PLAYER_IDLE_DURATION, 0, 0, false};

    animations_[static_cast<std::size_t>(State::Eat)] = {{
        toIntRect(constants::PLAYER_EAT_FRAME),
    }, constants::PLAYER_EATING_DURATION, 0, 0, false};

    animations_[static_cast<std::size_t>(State::Burp)] = {{
         toIntRect(constants::PLAYER_BURP_FRAME),
    }, constants::PLAYER_BURPING_DURATION, 0, 0, false};

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

    // Don't move while burping
    if (state_ == State::Burp)
    {
        updateFrame(deltaSeconds);
        applyFrame();
        return;
    }

    if (state_ != State::Eat)
    {
        if (movementDirection_ == 0.f)
        {
             setState(State::Idle);
        }
        else
        {
            setState(State::Run);
            facing_ = movementDirection_;
        }
    }

    updateFrame(deltaSeconds);
    sprite_.move({movementDirection_ * speed_ * deltaSeconds, 0.f});
    applyFrame();
    clampToWorld();
}

void Player::updateFrame(float dt)
{
    SpriteAnimation& anim = animations_[static_cast<std::size_t>(state_)];

    if (anim.time > anim.duration)
    {
        if (movementDirection_ == 0.f)
        {
             setState(State::Idle);
        }
        else
        {
            setState(State::Run);
            facing_ = movementDirection_;
        }

        return;
    }

    int totalFrames = anim.frames.size();
    const float frameDuration = anim.duration / totalFrames;

    anim.time += dt;
    if (anim.time > frameDuration * (anim.frame + 1))
    {
        anim.frame = (anim.frame + 1) % totalFrames;
        if (anim.frame == 0 && anim.loop) anim.time = 0;
    }
}

void Player::applyFrame()
{
    const SpriteAnimation& anim = animations_[static_cast<std::size_t>(state_)];
    const sf::IntRect& frame = anim.frames[anim.frame];
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

void Player::setState(State state)
{
    if (state_ == state) return;
    state_ = state;

    animations_[static_cast<std::size_t>(state_)].frame = 0;
    animations_[static_cast<std::size_t>(state_)].time = 0.f;
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
    setState(State::Eat);
}

void Player::startBurping()
{
    if (burps_ <= 0)
    {
        return;
    }

    --burps_;
    setState(State::Burp);
}

void Player::reset()
{
    setState(State::Idle);
    movementDirection_ = 0.f;
    facing_ = 1.f;
    burps_ = 1;
    sprite_.setPosition(startPosition_);
    applyFrame();
}

int Player::burps() const
{
    return burps_;
}

Player::State Player::state() const
{
    return state_;
}
}
