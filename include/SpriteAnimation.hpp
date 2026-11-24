#pragma once

#include <SFML/Graphics.hpp>

namespace bug_toss
{
class SpriteAnimation
{
public:
    std::vector<sf::IntRect> frames;
    float duration;
    size_t frame = 0;
    float time = 0;
    bool loop;
};
}