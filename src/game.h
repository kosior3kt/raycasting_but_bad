#ifndef __GAME__
#define __GAME__

#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_asyncio.h>

typedef int map_t[10][10];
typedef enum
{
	STATIC = 0,
	UP,
	DOWN,
	RIGHT,
	LEFT
} direction_t;

typedef struct vec
{
	int32_t x;
	int32_t y;
}Vector2;


typedef struct vec_d
{
	double x;
	double y;
}Vector2_d;

typedef struct
{
	int texture;
	double distance;
}texture_with_distance;

typedef struct
{
	Vector2_d* points;
	int* texture;
	double* distance;
	size_t size;
}camera_plane_t;


typedef struct player
{
	int32_t x;
	int32_t y;
	Vector2 camera_direction;
}player_t;

void run_game();

/*
*/
double get_distance(Vector2, Vector2);
void render_circles(SDL_Renderer**);
void render_one_line(SDL_Renderer**, const SDL_MouseMotionEvent, const player_t);
void render_starting_point(SDL_Renderer**);
void render_lines(SDL_Renderer**);

void draw_circle(SDL_Renderer**, const int32_t, const int32_t, const int32_t, const bool);


#endif  // __GAME__
