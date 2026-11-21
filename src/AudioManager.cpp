#include "AudioManager.hpp"

namespace bug_toss
{
AudioManager::AudioManager()
    : damageSound_(damageBuffer_), victorySound_(victoryBuffer_)
{
    // Load game background music
    if (!gameMusic_.openFromFile("assets/sounds/09 - Under the Stars.mp3"))
    {
        throw std::runtime_error("Failed to load background music");
    }
    gameMusic_.setLooping(true);
    gameMusic_.setVolume(70.f);

    // Load start screen music
    if (!startScreenMusic_.openFromFile("assets/sounds/03 - Hoo Hah.mp3"))
    {
        throw std::runtime_error("Failed to load start screen music");
    }
    startScreenMusic_.setLooping(true);
    startScreenMusic_.setVolume(70.f);

    // Load sound effects
    if (!eatBuffer_.loadFromFile("assets/sounds/pumba_gulp.mp3"))
    {
        throw std::runtime_error("Failed to load eat sound effect");
    }
   
    for (std::size_t i = 0; i < EAT_SOUND_POOL_SIZE_; i++)
    {
        eatSounds_.emplace_back(eatBuffer_);
    }

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

void AudioManager::playMusic(MusicTrack music)
{
    switch (music)
    {
        case MusicTrack::Playing:
            if (gameMusic_.getStatus() != sf::Music::Status::Playing)
            {
                gameMusic_.play();
            }
            break;
        case MusicTrack::Start:
            if (startScreenMusic_.getStatus() != sf::Music::Status::Playing)
            {
                startScreenMusic_.play();
            }
            break;
    } 
}

void AudioManager::stopMusic(MusicTrack music)
{
    switch (music)
    {
        case MusicTrack::Playing:
            gameMusic_.stop();
            break;
        case MusicTrack::Start:
            startScreenMusic_.stop();
            break;
    } 
}

void AudioManager::playEatSound()
{
    for (auto& sound : eatSounds_)
    {
        if (sound.getStatus() == sf::Sound::Status::Stopped)
        {
            sound.play();
            return;
        }
    }
}

void AudioManager::playSound(SoundEffect effect)
{
    switch (effect)
    {
        case SoundEffect::Eat:
            playEatSound();
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