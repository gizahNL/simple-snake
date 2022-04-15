/*
    SPDX-FileCopyrightText: 2022 Gijs Peskens gijs@peskens.net
    SPDX-FileCopyrightText: 2022 Luca Guspini
    SPDX-License-Identifier: MIT
*/

#include "SDL_events.h"
#include "SDL_render.h"
#include "SDL_surface.h"
#include "SDL_ttf.h"
#include "SDL_video.h"
#include <cstdlib>
#include <vector>
#include <cstdint>
#include <random>
#include <deque>
#include <queue>

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
    NONE = 0,
    UP = -1,
    DOWN = 1,
    LEFT = -2,
    RIGHT = 2,
};

struct PlayerState {
    int playeridx;
    SnakeDirection direction;
    std::queue<SnakeDirection> queuedCommands;
    std::deque<SnakeSegment> snake;
};

struct WorldState {
    int32_t worldX;
    int32_t worldY;
    Apple apple;
    bool paused;
    bool gameOver;
    bool player1Died;
    bool player2Died;
    bool isSingle;
};

class Game {
private:
    SDL_Window *window;
    SDL_Surface *windowSurface;
    TTF_Font *font;
    SDL_Surface *gameover;
    SDL_Surface *appleTex;
    SDL_Surface *player1Wins;
    SDL_Surface *player2Wins;
    SDL_Surface *tie;
    int32_t windowXsize;
    int32_t windowYsize;
    WorldState state;
    std::random_device rd;
    std::mt19937 mtGen;
    std::uniform_int_distribution<int32_t> dist;
    PlayerState player1;
    PlayerState player2;

    void Draw();
    void PlaceApple();
    void SetupWorldState(int32_t worldX, int32_t worldY);
    void ProcessInput(SDL_Event& event);
    auto PlayerTick(PlayerState& self);
    auto DoCollisions();
    auto CheckSelfCollision(const PlayerState& self);
    auto CheckOtherCollision(const PlayerState &self, const PlayerState &other);

public:
    Game();
    void Tick();
    void Loop();
};

}