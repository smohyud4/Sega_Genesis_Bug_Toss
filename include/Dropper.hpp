#pragma once

#include <SFML/Graphics.hpp>

#include <random>
#include <vector>

namespace bug_toss
{
class Dropper
{
public:
    Dropper(const sf::Texture& texture, float speed, std::mt19937& randomEngine,
            std::vector<sf::Vector2f> path);

    void update(sf::Time dt);
    void draw(sf::RenderTarget& target) const;
    void reset();

    [[nodiscard]] sf::Vector2f position() const;

private:
    void move(float dt);

    const std::vector<sf::Vector2f> path_;
    sf::Sprite sprite_;
    float speed_;
    std::mt19937& randomEngine_;
    int targetIndex_ = 1;
    float turnDistance_;
    float direction_ = 1.f;
    float distanceTraveled_ = 0.f;
};
}
