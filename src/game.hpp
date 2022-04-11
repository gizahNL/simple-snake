/*
    SPDX-FileCopyrightText: 2022 Gijs Peskens gijs@peskens.net
    SPDX-FileCopyrightText: 2022 Luca Guspini
    SPDX-License-Identifier: MIT
*/

#include "SDL_render.h"
#include "SDL_surface.h"
#include "SDL_ttf.h"
#include "SDL_video.h"
#include <cstdlib>
#include <vector>
#include <cstdint>
#include <random>
#include <deque>

namespace SnakeGame {

struct SnakeSegment {
    int32_t posX;
    int32_t posY;
};

struct Apple {
    int32_t posX;
    int32_t posY;
};

enum class SnakeDirection {
    UP,
    DOWN,
    LEFT,
    RIGHT,
};

struct WorldState {
    int32_t worldX;
    int32_t worldY;
    SnakeDirection direction;
    std::deque<SnakeSegment> snake;
    Apple apple;
    bool paused;
    bool gameOver;
    bool score;
};

class Game {
private:
    SDL_Window *window;
    SDL_Surface *windowSurface;
    TTF_Font *font;
    SDL_Surface *gameover;
    int32_t windowXsize;
    int32_t windowYsize;
    WorldState state;
    std::random_device rd;
    std::mt19937 mtGen;
    std::uniform_int_distribution<int32_t> dist;

    void Draw();
    void PlaceApple();
    void SetupWorldState(int32_t worldX, int32_t worldY);
public:
    Game();
    void Tick();
    void Loop();
};

}