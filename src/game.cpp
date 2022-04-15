/*
    SPDX-FileCopyrightText: 2022 Gijs Peskens gijs@peskens.net
    SPDX-FileCopyrightText: 2022 Luca Guspini
    SPDX-License-Identifier: MIT
*/

#include "game.hpp"
#include "SDL_events.h"
#include "SDL_keycode.h"
#include "SDL_pixels.h"
#include "SDL_rect.h"
#include "SDL_rwops.h"
#include "SDL_scancode.h"
#include "SDL_stdinc.h"
#include "SDL_surface.h"
#include "SDL_timer.h"
#include "SDL_ttf.h"
#include "SDL_video.h"
#include "SDL2/SDL_image.h"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace SnakeGame {

constexpr int maxQueuedCommands = 3;

void addToQueue(std::queue<SnakeDirection> &queue, SnakeDirection direction)
{
    if (queue.size() >= maxQueuedCommands)
        return;
    queue.push(direction);
}

void Game::Draw()
{
    SDL_FillRect(windowSurface, NULL, SDL_MapRGB(windowSurface->format, 0, 0, 0));
    auto XstepSize = windowXsize / state.worldX;
    auto YstepSize = windowYsize / state.worldY;
    auto rIdx = 0;
    std::vector<SDL_Rect> rects(player1.snake.size());
    for (const auto &segment : player1.snake)
    {
        SDL_Rect pos;
        pos.x = segment.posX * XstepSize;
        pos.y = segment.posY * YstepSize;
        pos.h = YstepSize;
        pos.w = XstepSize;
        rects[rIdx++] = pos;

        //SDL_FillRect(windowSurface, &pos, SDL_MapRGB(windowSurface->format, 255, 255, 255));
    }
    SDL_FillRects(windowSurface, rects.data(), rects.size(), SDL_MapRGBA(windowSurface->format, 50, 255, 50, 128));
    if (!state.isSingle) {
        rIdx = 0;
        rects.resize(player2.snake.size());
        for (const auto &segment : player2.snake)
        {
            SDL_Rect pos;
            pos.x = segment.posX * XstepSize;
            pos.y = segment.posY * YstepSize;
            pos.h = YstepSize;
            pos.w = XstepSize;
            rects[rIdx++] = pos;
        }
        SDL_FillRects(windowSurface, rects.data(), rects.size(), SDL_MapRGBA(windowSurface->format, 192, 50, 50, 128));
    }

    SDL_Rect apple;
    apple.x = state.apple.posX * XstepSize;
    apple.y = state.apple.posY * YstepSize;
    apple.w = XstepSize;
    apple.h = YstepSize;
    //SDL_FillRect(windowSurface, &apple, SDL_MapRGB(windowSurface->format, 255, 0, 10));
    SDL_BlitScaled(appleTex, NULL, windowSurface, &apple);
    if (state.gameOver) {
        char score[256];
        if (state.isSingle)
            snprintf(score, 256, "Score: %zu", player1.snake.size());
        else
            snprintf(score, 256, "Player 1 score: %zu", player1.snake.size());
        SDL_Color sCol;
        sCol.a = 255;
        sCol.r = 255;
        sCol.g = 255;
        sCol.b = 255;
        SDL_Surface *scoreSurface = TTF_RenderText_Blended(font, score, sCol);
        SDL_Rect scoreRect;
        if (state.isSingle)
            scoreRect.x = 50;
        else
            scoreRect.x = 5;
        scoreRect.y = 400;
        scoreRect.w = 200;
        scoreRect.h = 50;

        SDL_BlitScaled(scoreSurface, NULL, windowSurface, &scoreRect);
        SDL_FreeSurface(scoreSurface);

        SDL_Rect goRect;
        goRect.x = 200;
        goRect.y = 250;
        goRect.w = 200;
        goRect.h = 50;
        if (state.isSingle) {
            SDL_BlitScaled(gameover, NULL, windowSurface, &goRect);
        } else  {
            snprintf(score, 256, "Player 2 score: %zu", player2.snake.size());
            scoreSurface = TTF_RenderText_Blended(font, score, sCol);
            scoreRect.x = 305;
            SDL_BlitScaled(scoreSurface, NULL, windowSurface, &scoreRect);
            SDL_FreeSurface(scoreSurface);
            if (state.player1Died && state.player2Died)
                SDL_BlitScaled(tie, NULL, windowSurface, &goRect);
            else if (state.player1Died)
                SDL_BlitScaled(player2Wins, NULL, windowSurface, &goRect);
            else
                SDL_BlitScaled(player1Wins, NULL, windowSurface, &goRect);

        }
    }
    SDL_UpdateWindowSurface(window);
}

void Game::PlaceApple()
{
    int32_t y, x;

    x = dist(mtGen);
    y = dist(mtGen);

    //CHECK DOESNT COLLIDE SNAKE

    state.apple.posX = x;
    state.apple.posY = y;
}

auto Game::PlayerTick(PlayerState &player)
{
    if (!player.queuedCommands.empty()) {
        auto newDirection = player.queuedCommands.front();
        if (((int)newDirection * -1) != (int)player.direction)
            player.direction = newDirection;
        player.queuedCommands.pop();
    }
    SnakeSegment newHead;
    newHead.posX = player.snake[0].posX;
    newHead.posY = player.snake[0].posY;

    switch(player.direction) {
    case SnakeDirection::NONE:
        player.direction = SnakeDirection::UP;
    case SnakeDirection::UP:
        newHead.posY -= 1;
        break;
    case SnakeDirection::DOWN:
        newHead.posY += 1;
        break;
    case SnakeDirection::RIGHT:
        newHead.posX += 1;
        break;
    case SnakeDirection::LEFT:
        newHead.posX -= 1;
        break;
    }

    if (newHead.posX > state.worldX)
        newHead.posX = 0;
    else if (newHead.posX < 0)
        newHead.posX = state.worldX;
    else if (newHead.posY > state.worldY)
        newHead.posY = 0;
    else if (newHead.posY < 0)
        newHead.posY = state.worldY;

    player.snake.push_front(newHead);
    bool appleHit = (newHead.posX == state.apple.posX && newHead.posY == state.apple.posY);
    if (!appleHit)
        player.snake.pop_back();
    return appleHit;
}

auto Game::CheckSelfCollision(const PlayerState& self)
{
    const auto& newHead = self.snake[0];
    for (size_t i = 1; i < self.snake.size(); i++)
    {
        const auto& segment = self.snake[i];
        if (segment.posX == newHead.posX && segment.posY == newHead.posY) {
            return true;
        }
    }
    return false;
}

auto Game::CheckOtherCollision(const PlayerState &self, const PlayerState &other)
{
    const auto& newHead = self.snake[0];
    for (const auto& segment : other.snake)
    {
        if (newHead.posX == segment.posX && newHead.posY == segment.posY)
            return true;
    }
    return false;
}

auto Game::DoCollisions()
{
    if (state.isSingle)
    {
        if (CheckSelfCollision(player1))
            state.gameOver = true;
        return;
    }
    state.player1Died = (CheckSelfCollision(player1) || CheckOtherCollision(player1,player2));
    state.player2Died = (CheckSelfCollision(player2) || CheckOtherCollision(player2, player1));
    state.gameOver = (state.player1Died || state.player2Died);
}

void Game::Tick()
{
    if (!state.paused) {
        bool appleHit = false;
        if (state.isSingle) {
            appleHit = PlayerTick(player1);
        } else {
            auto p1hit = PlayerTick(player1);
            auto p2hit = PlayerTick(player2);
            appleHit = (p1hit || p2hit);
        }

        if (appleHit)
            PlaceApple();

        DoCollisions();
    }
}

void Game::ProcessInput(SDL_Event& event)
{
    if (state.isSingle && player1.queuedCommands.size() < maxQueuedCommands) {
        switch (event.key.keysym.sym)
        {
        case SDLK_LEFT:  
        case SDLK_a:
            player1.queuedCommands.push(SnakeDirection::LEFT);
            break;
        case SDLK_RIGHT:
        case SDLK_d:
            player1.queuedCommands.push(SnakeDirection::RIGHT);
            break;
        case SDLK_UP:
        case SDLK_w:
            player1.queuedCommands.push(SnakeDirection::UP);
            break;
        case SDLK_DOWN:
        case SDLK_s:
            player1.queuedCommands.push(SnakeDirection::DOWN);
            break;
        }
    } else if (!state.isSingle)
    {
        switch (event.key.keysym.sym)
        {
        //Player 1
        case SDLK_a:
            addToQueue(player1.queuedCommands, SnakeDirection::LEFT);
            break;
        case SDLK_d:
            addToQueue(player1.queuedCommands, SnakeDirection::RIGHT);
            break;
        case SDLK_w:
            addToQueue(player1.queuedCommands, SnakeDirection::UP);
            break;
        case SDLK_s:
            addToQueue(player1.queuedCommands, SnakeDirection::DOWN);
            break;
        //Player 2
        case SDLK_LEFT:
            addToQueue(player2.queuedCommands, SnakeDirection::LEFT);
            break;
        case SDLK_RIGHT:
            addToQueue(player2.queuedCommands, SnakeDirection::RIGHT);
            break;
        case SDLK_UP:
            addToQueue(player2.queuedCommands, SnakeDirection::UP);
            break;
        case SDLK_DOWN:
            addToQueue(player2.queuedCommands, SnakeDirection::DOWN);
            break;        
        }
    }
}

void Game::Loop()
{
    bool keepScreenOpen = true;
    SDL_Event windowEvent;
    std::chrono::time_point<std::chrono::steady_clock> lastPause;
    auto nextTick = std::chrono::steady_clock::now() + std::chrono::milliseconds(100);
    auto nextDraw = std::chrono::steady_clock::now();
    while (keepScreenOpen)
    {
        while (SDL_PollEvent(&windowEvent) >0)
        {
            switch(windowEvent.type)
            {
            case SDL_QUIT:
                keepScreenOpen = false;
                break;
            case SDL_KEYDOWN:
                auto wasPaused = state.paused;
                Uint8 const *keys = SDL_GetKeyboardState(nullptr);
                if (keys[SDL_SCANCODE_SPACE] != 1)
                    state.paused = false;
                if(keys[SDL_SCANCODE_M] == 1 && state.isSingle == true)
                    state.gameOver = 1;
                else if(keys[SDL_SCANCODE_SPACE] == 1) {
                    if (state.gameOver) {
                        player1.snake.resize(0);
                        player2.snake.resize(0);
                        SetupWorldState(state.worldX, state.worldY);
                        state.gameOver = false;
                    } else if (!state.paused) {
                        auto now = std::chrono::steady_clock::now();
                        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastPause).count() < 2) {
                            //HAHA NO PAUSE
                        } else {
                            state.paused = true;
                            lastPause = now;
                        }
                    } else {
                        state.paused = false;
                    }
                }
                if (wasPaused && !state.paused) {
                    std::queue<SnakeDirection> empty;
                    std::swap(player1.queuedCommands, empty);
                }
                ProcessInput(windowEvent);
                break;
            }
        }
        auto now = std::chrono::steady_clock::now();
        if (!state.paused && !state.gameOver && now >= nextTick ) {
            Tick();
            float delayFactor = log(player1.snake.size());
            int32_t delay = 100 - (delayFactor * 8);
            delay = std::max(delay, (int32_t)45);
            //fprintf(stderr, "Ticktime is: %d\n", delay);
            //fprintf(stderr, "Score is %zu\n", player1.snake.size());
            nextTick = now += std::chrono::milliseconds(delay);
        }
        if (now >= nextDraw) {
            Draw();
            nextDraw += std::chrono::milliseconds(20);
        }
    }
}

void Game::SetupWorldState(int32_t worldX, int32_t worldY)
{
    state.worldX = worldX;
    state.worldY = worldY;
    PlaceApple();
    state.paused = true;
    state.isSingle = false;
    state.gameOver = false;
    if (state.isSingle) {
        player1.snake.push_back({state.worldX/2, state.worldY/2});
        player1.direction = SnakeDirection::NONE;
    } else {
        auto Xstep = state.worldX /3;
        player1.snake.push_back({Xstep, state.worldY/2});
        player1.snake.push_back({Xstep, state.worldY/2});
        player1.snake.push_back({Xstep, state.worldY/2});
        player1.direction = SnakeDirection::NONE;
        player2.snake.push_back({Xstep *2, state.worldY/2});
        player2.snake.push_back({Xstep *2, state.worldY/2});
        player2.snake.push_back({Xstep *2, state.worldY/2});
        player2.direction = SnakeDirection::NONE;
    }
}

Game::Game() : 
        mtGen(rd())
    {

    windowXsize = 600;
    windowYsize = 600;

    window = SDL_CreateWindow("Simple Snake",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        windowXsize, windowYsize,
        0);

    if (!window) {
        fprintf(stderr, "Couldn't create window: %s\n", SDL_GetError());
        throw std::runtime_error("error creating window");
    }

    windowSurface = SDL_GetWindowSurface(window);

    if (!windowSurface) {
        fprintf(stderr, "Couldn't get a window surface %s\n", SDL_GetError());
        throw std::runtime_error("error surface for window");
    }

    font = TTF_OpenFont("DejaVuSans.ttf", 72);

    if (!font) {
        fprintf(stderr, "Couldn't load font\n");
        throw std::runtime_error("Couldn't open font");
    }

    SDL_Color col;
    col.a = 255;
    col.r = 255;
    col.g = 0;
    col.b = 0;

    gameover = TTF_RenderText_Blended(font, "You loose!", col);
    if (!gameover) {
        fprintf(stderr, "Couldn't render game over text\n");
        throw std::runtime_error("failed to render gameover");
    }
    player1Wins = TTF_RenderText_Blended(font, "Player 1 WINS!", col);

    player2Wins = TTF_RenderText_Blended(font, "Player 2 WINS!", col);

    tie = TTF_RenderText_Blended(font, "You both suck!", col);
    
    appleTex = IMG_Load("apple.png");
    if (!appleTex) {
        fprintf(stderr, "Couldn't load apple.png %s\n", SDL_GetError());
        throw std::runtime_error("failed to load texture");
    }

    int32_t worldMax = 32;
    dist = std::uniform_int_distribution<int32_t>(0, worldMax);

    SetupWorldState(worldMax, worldMax);

    player1.playeridx = 1;
    player2.playeridx = 2;
    }
}