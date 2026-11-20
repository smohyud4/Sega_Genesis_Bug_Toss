#pragma once

#include <SFML/Audio.hpp>

namespace bug_toss
{
class AudioManager
{
public:
    enum class SoundEffect
    {
        Eat,
        Burp,
        Victory
    };

    AudioManager();

    void playMusic();
    void stopMusic();

    void playSound(SoundEffect effect);

private:
    // Background music
    sf::Music music_;

    // Sound effect resources
    sf::SoundBuffer eatBuffer_;
    sf::SoundBuffer damageBuffer_;
    sf::SoundBuffer victoryBuffer_;

    // Sound effect players
    sf::Sound eatSound_;
    sf::Sound damageSound_;
    sf::Sound victorySound_;
};
}