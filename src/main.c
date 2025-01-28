#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_asyncio.h>

// mine shit
#include "game.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

int main(int argc, char* argv[])
{
	run_game();
	return 0;
}

