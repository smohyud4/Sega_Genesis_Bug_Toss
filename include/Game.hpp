#pragma once

#include <SFML/Graphics.hpp>

#include "Bug.hpp"
#include "Dropper.hpp"
#include "Player.hpp"
#include "AnimatedText.hpp"
#include "AudioManager.hpp"

#include <random>
#include <vector>

namespace bug_toss
{
class Game
{
public:
    Game();

    void run();

private:
    enum class State
    {
        Start,
        Play,
        End
    };

    enum class Difficulty
    {
        Easy,
        Hard
    };

    void processEvents();
    void update(sf::Time dt);
    void render();
    void updateViewport(sf::Vector2u windowSize);
    void startGame();
    void endGame();
    void spawnBug();
    [[nodiscard]] float nextSpawnDelay();
    void centerText(sf::Text& text, sf::Vector2f position) const;
    void handleMenuActivation(sf::Vector2f position);

    sf::RenderWindow window_;

    sf::View view_;

    AudioManager audioManager_;

    sf::Texture backgroundTexture_;
    sf::RectangleShape background_;

    sf::Texture startBackgroundTexture_;
    sf::RectangleShape startBackground_;
    sf::Font menuFont_;
    sf::Text playText_;
    AnimatedText scoreText_;

    sf::Texture bugTexture_;
    sf::Texture timonTexture_;
    sf::Texture pumbaTexture_;

    std::mt19937 randomEngine_;
    std::uniform_int_distribution<int> bugPick_;
    std::uniform_int_distribution<int> gravityPick_;
    std::uniform_real_distribution<float> spawnDelayPick_;

    Player player_;
    Dropper dropper_;
    std::vector<Bug> bugs_;
    int burpAttempts_ = 1;
    int score_ = 0;
    int difficultyThreshold_;
    State state_ = State::Start;
    Difficulty difficulty_ = Difficulty::Easy;
    float spawnTimer_ = 0.f;
};
}
