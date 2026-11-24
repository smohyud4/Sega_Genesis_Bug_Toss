#pragma once

#include <SFML/Graphics.hpp>
#include "SpriteAnimation.hpp"

#include <array>

namespace bug_toss
{
class Player
{
public:
    enum class State
    {
        Idle,
        Run,
        Eat,
        Burp
    };

    Player(const sf::Texture& texture, float speed, sf::Vector2f startPosition);

    void setMovementDirection(float direction);
    void update(sf::Time dt);
    void draw(sf::RenderTarget& target) const;
    void startEating();
    void startBurping();
    void reset();
    State state() const;

    [[nodiscard]] sf::FloatRect bounds() const;
    [[nodiscard]] float movementSpeed() const;

private:

    void updateFrame(float dt);
    void applyFrame();
    void clampToWorld();
    void setState(State state);

    std::array<SpriteAnimation, 4> animations_;
    const sf::Vector2f startPosition_;
    sf::Sprite sprite_;
    State state_ = State::Idle;
    float movementDirection_ = 0.f;
    float facing_ = 1.f;
    float speed_;
};
}
