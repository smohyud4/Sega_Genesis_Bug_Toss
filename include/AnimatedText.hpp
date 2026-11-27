#pragma once

#include <SFML/Graphics.hpp>

namespace bug_toss
{
class AnimatedText
{
public:
    AnimatedText(const sf::Font& font, unsigned int size, int totalScore, float duration);

    void update(sf::Time dt);
    void draw(sf::RenderTarget& target) const;
    void reset();
    void setTotalScore(int score);
    int currScore() const;
    sf::Text& text();
private:
    float time_ = 0.f;
    int score_ = 0;
    float duration_;
    int totalScore_;
    sf::Text text_;
};
}