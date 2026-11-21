#pragma once

#include <SFML/Graphics.hpp>

namespace bug_toss
{
class Bug
{
public:
    Bug(const sf::Texture& texture, sf::Vector2f startPosition, float gravity, int frameIndex);

    void update(sf::Time dt);
    void draw(sf::RenderTarget& target) const;

    [[nodiscard]] sf::Vector2f position() const;
    [[nodiscard]] sf::FloatRect bounds() const;
    [[nodiscard]] bool isDeadly() const;

private:
    sf::Sprite sprite_;
    float gravity_;
    float speed_ = 0.f;
    bool isDeadly_;
};
}
