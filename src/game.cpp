/*
    SPDX-FileCopyrightText: 2022 Gijs Peskens gijs@peskens.net
    SPDX-FileCopyrightText: 2022 Luca Guspini
    SPDX-License-Identifier: MIT
*/

#include "game.hpp"
#include "SDL_events.h"
#include "SDL_pixels.h"
#include "SDL_rect.h"
#include "SDL_scancode.h"
#include "SDL_surface.h"
#include "SDL_timer.h"
#include "SDL_ttf.h"
#include "SDL_video.h"
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <stdexcept>

namespace SnakeGame {

void Game::Draw()
{
    SDL_FillRect(windowSurface, NULL, SDL_MapRGB(windowSurface->format, 0, 0, 0));
    auto XstepSize = windowXsize / state.worldX;
    auto YstepSize = windowYsize / state.worldY;
    for (const auto &segment : state.snake)
    {
        SDL_Rect pos;
        pos.x = segment.posX * XstepSize;
        pos.y = segment.posY * YstepSize;
        pos.h = YstepSize;
        pos.w = XstepSize;

        SDL_FillRect(windowSurface, &pos, SDL_MapRGB(windowSurface->format, 255, 255, 255));
    }

    SDL_Rect apple;
    apple.x = state.apple.posX * XstepSize;
    apple.y = state.apple.posY * YstepSize;
    apple.w = XstepSize;
    apple.h = YstepSize;
    SDL_FillRect(windowSurface, &apple, SDL_MapRGB(windowSurface->format, 255, 0, 10));

    if (state.gameOver) {
        char score[256];
        snprintf(score, 256, "Score: %d", state.score);

        SDL_Color sCol;
        sCol.a = 255;
        sCol.r = 255;
        sCol.g = 255;
        sCol.b = 255;
        //SDL_Surface *scoreSurface = TTF_RenderText_Blended(font, score, sCol);

        //SDL_BlitScaled(scoreSurface, NULL, windowSurface, );

        SDL_Rect goRect;
        goRect.x = 300;
        goRect.y = 300;
        goRect.w = 200;
        goRect.h = 50;
        SDL_BlitScaled(gameover, NULL, windowSurface, &goRect);
    }
    SDL_UpdateWindowSurface(window);
}

void Game::PlaceApple()
{
    int32_t y, x;

    y = dist(mtGen);
    x = dist(mtGen);

    //CHECK DOESNT COLLIDE SNAKE

    state.apple.posX = x;
    state.apple.posY = y;
}

void Game::Tick()
{
    if (!state.paused) {
        SnakeSegment newHead;
        newHead.posX = state.snake[0].posX;
        newHead.posY = state.snake[0].posY;
        switch(state.direction) {
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

        bool appleHit = (newHead.posX == state.apple.posX && newHead.posY == state.apple.posY);
        if (appleHit) {
            PlaceApple();
            state.score = state.score + 1;
        } else {
            state.snake.pop_back();
        }

        for (const auto &segment : state.snake)
        {
            if (segment.posX == newHead.posX && segment.posY == newHead.posY) {
                state.gameOver = true;
                break;
            }
        }

        state.snake.push_front(newHead);

    }
}

void Game::Loop()
{
    bool keepScreenOpen = true;
    SDL_Event windowEvent;
    std::chrono::time_point<std::chrono::steady_clock> lastPause;
    auto nextTick = std::chrono::steady_clock::now() + std::chrono::milliseconds(100);
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
                Uint8 const *keys = SDL_GetKeyboardState(nullptr);
                if (keys[SDL_SCANCODE_SPACE] != 1)
                    state.paused = false;
                if(state.direction != SnakeDirection::DOWN && (keys[SDL_SCANCODE_W] == 1 || keys[SDL_SCANCODE_UP] == 1))
                    state.direction = SnakeDirection::UP;
                else if(state.direction != SnakeDirection::UP && (keys[SDL_SCANCODE_S] == 1 || keys[SDL_SCANCODE_DOWN] == 1))
                    state.direction = SnakeDirection::DOWN;
                else if(state.direction != SnakeDirection::RIGHT && (keys[SDL_SCANCODE_A] == 1 || keys[SDL_SCANCODE_LEFT] == 1))
                    state.direction = SnakeDirection::LEFT;
                else if(state.direction != SnakeDirection::LEFT && (keys[SDL_SCANCODE_D] == 1 || keys[SDL_SCANCODE_RIGHT] == 1))
                    state.direction = SnakeDirection::RIGHT;
                else if(keys[SDL_SCANCODE_SPACE] == 1) {
                    if (state.gameOver) {
                        state.snake.resize(0);
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
                break;
            }
        }
        auto now = std::chrono::steady_clock::now();
        if (!state.paused && !state.gameOver && now >= nextTick ) {
            Tick();
            nextTick = now += std::chrono::milliseconds(125);
        }
        Draw();
    }
}

void Game::SetupWorldState(int32_t worldX, int32_t worldY)
{
    state.worldX = worldX;
    state.worldY = worldY;
    state.direction = SnakeDirection::UP;
    state.snake.push_back({state.worldX/2, state.worldY/2});
    state.paused = true;
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

    gameover = TTF_RenderUTF8_Blended(font, "You loose!", col);
    if (!gameover) {
        fprintf(stderr, "Couldn't render game over text\n");
        throw std::runtime_error("failed to render gameover");
    }

    int32_t worldMax = 32;
    SetupWorldState(worldMax, worldMax);

    dist = std::uniform_int_distribution<int32_t>(0, worldMax);

    PlaceApple();
}

}