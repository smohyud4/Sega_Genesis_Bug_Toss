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

    enum class MusicTrack
    {
        Start,
        Playing
    };

    AudioManager();

    void playMusic(MusicTrack music);
    void stopMusic(MusicTrack music);

    void playSound(SoundEffect effect);

private:
    void playEatSound();
    // Background music
    sf::Music gameMusic_;
    sf::Music startScreenMusic_;

    // Sound effect resources
    sf::SoundBuffer eatBuffer_;
    sf::SoundBuffer burpBuffer_;
    sf::SoundBuffer victoryBuffer_;

    // Sound effect players
    sf::Sound burpSound_;
    sf::Sound victorySound_;

    std::vector<sf::Sound> eatSounds_;

    static constexpr std::size_t EAT_SOUND_POOL_SIZE_ = 3;
};
}