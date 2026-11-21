#pragma once

#include <bitset>
#include <array>

namespace bug_toss::constants
{
inline constexpr unsigned int INTERNAL_WIDTH = 320;
inline constexpr unsigned int INTERNAL_HEIGHT = 224;
inline constexpr unsigned int INITIAL_WINDOW_SCALE = 3;
inline constexpr float PIXEL_SCALE = 1.f;
inline constexpr float DROPPER_SPEED = 250.f;
inline constexpr float PLAYER_SPEED = 300.f;
inline constexpr float BUG_BOTTOM = 200.f;
inline constexpr std::size_t MAX_BUGS = 4;
inline constexpr float MINIMUM_SPAWN_DELAY = 0.20f;
inline constexpr float MAXIMUM_SPAWN_DELAY = 1.5f;
inline constexpr int MEDIUM_DIFFICULTY_THRESHOLD = 30;
inline constexpr int HARD_DIFFICULTY_THRESHOLD = 60;
inline constexpr unsigned int MENU_TEXT_SIZE = 24;
inline constexpr float TEXT_ANIMATION_DURATION = 1.f;
inline constexpr std::array<float, 3> BUG_GRAVITIES = {75.f, 75.f * 1.5f, 75.f * 2.25f};

struct DifficultyScale
{
    float spawnDelayMultiplier;
    float gravityMultiplier;
};

inline constexpr std::array<DifficultyScale, 3> DIFFICULTY_SCALES{{
    {1.f, 1.1f},
    {0.9f, 1.25f},
    {0.9f, 1.4f}
}};
inline constexpr float DROPPER_STEP_DISTANCES[] = {
    600.f / 3.f, 1000.f / 3.f, 1200.f / 3.f
};
inline constexpr float PLAYER_EATING_DURATION = 0.12f;
inline constexpr float PLAYER_BURPING_DURATION = 1.5f;
inline constexpr float PLAYER_START_X = 160.f;
inline constexpr float PLAYER_START_Y = 650.f / 3.f;

struct Frame
{
    int x;
    int y;
    int width;
    int height;
};

struct Point
{
    float x;
    float y;
};

inline constexpr std::array<Frame, 15> BUG_FRAMES{{
    {3, 1, 16, 20}, {26, 1, 16, 20}, {50, 1, 19, 20},
    {73, 1, 19, 20}, {95, 1, 19, 20}, {3, 26, 19, 19},
    {26, 26, 21, 19}, {52, 26, 19, 19}, {76, 26, 17, 19},
    {99, 26, 17, 19}, {4, 47, 17, 23}, {27, 47, 17, 23},
    {49, 47, 19, 23}, {71, 47, 19, 23}, {95, 47, 23, 23}
}};

inline const std::bitset<15> BUG_DEADLINESS("100000000000000");

inline constexpr Frame DROPPER_FRAME{15, 104, 52, 37};
inline constexpr Frame PLAYER_IDLE_FRAME{13, 10, 33, 38};
inline constexpr Frame PLAYER_RUN_FRAME{324, 91, 51, 42};
inline constexpr Frame PLAYER_EAT_FRAME{4, 148, 35, 45};
inline constexpr Frame PLAYER_BURP_FRAME{366, 148, 35, 45};

inline constexpr std::array<Point, 8> DROPPER_PATH{{
    {64.f * 0.4f, 263.f * (1.12f / 3.f)},
    {157.f * 0.4f, 204.f * (1.12f / 3.f)},
    {238.f * 0.4f, 179.f * (1.12f / 3.f)},
    {269.f * 0.4f, 171.f * (1.12f / 3.f)},
    {652.f * 0.4f, 232.f * (1.12f / 3.f)},
    {683.f * 0.4f, 240.f * (1.12f / 3.f)},
    {708.f * 0.4f, 233.f * (1.12f / 3.f)},
    {738.f * 0.4f, 216.f * (1.12f / 3.f)}
}};
}
