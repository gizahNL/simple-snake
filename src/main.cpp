/*
    SPDX-FileCopyrightText: 2022 Gijs Peskens gijs@peskens.net
    SPDX-FileCopyrightText: 2022 Luca Guspini
    SPDX-License-Identifier: MIT
*/

#include "SDL_error.h"
#include "game.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <cstdio>

int main(int argc, char* argv[])
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		fprintf(stderr, "Couldn't init SDL: %s\n", SDL_GetError());
		return 1;
	}

	if (TTF_Init() != 0) {
		fprintf(stderr, "Couldn't init SDL TTF %s\n", SDL_GetError());
		return 1;
	}

	if (IMG_Init(0) != 0) {
		fprintf(stderr, "Couldn't init SDL Image %s\n", SDL_GetError());
		return 1;
	}

	try {
		SnakeGame::Game game;

		game.Loop();
		return 0;
	} catch(...) {
		return 1;
	}
}