#include "AnimatedText.hpp"

#include <iomanip>
#include <sstream>

namespace bug_toss
{
AnimatedText::AnimatedText(const sf::Font& font, unsigned int size, int totalScore, float duration)
    : duration_(duration),
      totalScore_(totalScore),
      text_(font, "BUGS COLLECTED 00", size)
{   
    text_.setFillColor(sf::Color(230, 227, 5));
    text_.setCharacterSize(18);
}

void AnimatedText::update(sf::Time dt)
{
    if (score_ == totalScore_) return;

    float seconds = dt.asSeconds();
    const float frameDuration = duration_ / totalScore_;

    time_ += seconds;
    if (time_ > frameDuration * (score_ + 1))
    {
        score_++;
    }

    std::stringstream ss;
    ss << "BUGS COLLECTED " << std::setfill('0') << std::setw(2) << score_;
    text_.setString(ss.str());
}

void AnimatedText::reset()
{
    time_ = 0.f;
    score_ = 0;
    text_.setString("BUGS COLLECTED 00");
}

void AnimatedText::setTotalScore(int score)
{
    totalScore_ = score;
}

void AnimatedText::draw(sf::RenderTarget& target) const
{
    target.draw(text_);
}

int AnimatedText::currScore() const
{
    return score_;
}

sf::Text& AnimatedText::text()
{
    return text_;
} 
}