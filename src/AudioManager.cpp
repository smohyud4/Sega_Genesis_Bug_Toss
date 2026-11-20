#include "AudioManager.hpp"

namespace bug_toss
{
AudioManager::AudioManager()
    : eatSound_(eatBuffer_), damageSound_(damageBuffer_), victorySound_(victoryBuffer_)
{
    // Load background music
    if (!music_.openFromFile("assets/sounds/09 - Under the Stars.mp3"))
    {
        throw std::runtime_error("Failed to load background music");
    }
    music_.setLooping(true);
    music_.setVolume(70.f);

    // Load sound effects
    if (!eatBuffer_.loadFromFile("assets/sounds/pumba_gulp.mp3"))
    {
        throw std::runtime_error("Failed to load eat sound effect");
    }
    eatSound_.setBuffer(eatBuffer_);

    if (!damageBuffer_.loadFromFile("assets/sounds/pumba_burp.mp3"))
    {
        throw std::runtime_error("Failed to load damage sound effect");
    }
    damageSound_.setBuffer(damageBuffer_);

    if (!victoryBuffer_.loadFromFile("assets/sounds/pumba_win.mp3"))
    {
        throw std::runtime_error("Failed to load victory sound effect");
    }
    victorySound_.setBuffer(victoryBuffer_);
}

void AudioManager::playMusic()
{
    music_.play();
}

void AudioManager::stopMusic()
{
    music_.stop();
}

void AudioManager::playSound(SoundEffect effect)
{
    switch (effect)
    {
        case SoundEffect::Eat:
            eatSound_.play();
            break;
        case SoundEffect::Burp:
            damageSound_.play();
            break;
        case SoundEffect::Victory:
            victorySound_.play();
            break;
    }
}
}