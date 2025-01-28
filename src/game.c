#include "game.h"
#include <stdint.h>
#include <stdlib.h>
#include <math.h>


/////static functions definitions
static bool update_and_render(SDL_Renderer**, SDL_Window**);
static void draw_map_and_grid(SDL_Renderer*, const map_t, const int32_t, const int32_t);
static bool key_pressed(SDL_KeyboardEvent, direction_t*, player_t*);
static void mouse_callback(SDL_MouseButtonEvent);
static bool is_a_wall(const Vector2, const map_t);
static void render_player(SDL_Renderer**, const player_t);
static void update_camera_direction(const SDL_MouseMotionEvent, player_t*);

//make these suckers const or sth
static void draw_square(SDL_Renderer**, int32_t, int32_t);
/////////end of definitions

static unsigned HEIGHT = 640;
static unsigned WIDTH = 640;

const int height = 50;
const int width  = 50;

const int num_col = 10;
const int num_row = 10;

static Vector2 coords[100];
static Vector2 starting_point;

const int g_map[10][10] =
{
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
	{0, 0, 1, 0, 0, 1, 0, 0, 0, 0},
	{0, 0, 1, 1, 1, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

int get_ingame_coord(int _coord)
{
	return floor(_coord / 50);
}

Vector2 get_ingame_coordinates(Vector2 _vec)
{
	_vec.x = _vec.x/width;
	_vec.y = _vec.y/height;
	return _vec;
}

player_t player;

static bool update_and_render(SDL_Renderer** _renderer, SDL_Window** _window)
{
	SDL_Event e;
	bool should_quit		= false;
	direction_t direction	= STATIC;

	while (SDL_PollEvent(&e) != 0)
	{
		SDL_SetRenderDrawColor(*_renderer, 9, 0, 0, 0);
		SDL_RenderClear(*_renderer);
		draw_map_and_grid(*_renderer, g_map, 10, 10);

		switch(e.type)
		{
			case SDL_EVENT_QUIT:
				should_quit = true;
				break;
			case SDL_EVENT_KEY_DOWN:
				should_quit = key_pressed(*(SDL_KeyboardEvent*)&e,
						&direction, &player);
				break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				mouse_callback(*(SDL_MouseButtonEvent*)&e);
				break;
			case SDL_EVENT_MOUSE_MOTION:
				update_camera_direction(*(SDL_MouseMotionEvent*)&e, &player);
				break;
		}

		render_player(_renderer, player);

		SDL_RenderPresent(*_renderer);
	}

	return should_quit;
}

void draw_circle(SDL_Renderer** _renderer,
				const int32_t _centreX, const int32_t _centreY,
				const int32_t _radius, const bool _draw_text)
{
	const int32_t diameter = (_radius * 2);

	int32_t x = (_radius - 1);
	int32_t y = 0;
	int32_t tx = 1;
	int32_t ty = 1;
	int32_t error = (tx - diameter);

	while (x >= y)
	{
		//  Each of the following renders an octant of the circle
		SDL_RenderPoint(*_renderer, _centreX + x, _centreY - y);
		SDL_RenderPoint(*_renderer, _centreX + x, _centreY + y);
		SDL_RenderPoint(*_renderer, _centreX - x, _centreY - y);
		SDL_RenderPoint(*_renderer, _centreX - x, _centreY + y);
		SDL_RenderPoint(*_renderer, _centreX + y, _centreY - x);
		SDL_RenderPoint(*_renderer, _centreX + y, _centreY + x);
		SDL_RenderPoint(*_renderer, _centreX - y, _centreY - x);
		SDL_RenderPoint(*_renderer, _centreX - y, _centreY + x);

		if (error <= 0)
		{
			++y;
			error += ty;
			ty += 2;
		}

		if (error > 0)
		{
			--x;
			tx += 2;
			error += (tx - diameter);
		}
	}

	if(_draw_text == true)
	{
		char buff[64];
		sprintf(buff, "%d, %d", _centreX, _centreY);
		SDL_RenderDebugText(*_renderer, _centreX - 15, _centreY - 30, buff);
	}
}

double get_distance(Vector2 p1, Vector2 p2)
{
	return sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
}

double get_distance_d(Vector2_d p1, Vector2_d p2)
{
	return sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
}

static bool is_outside_map(const Vector2 _point)
{
	if(_point.x >= width * num_col  || _point.x <= 0) return true;
	if(_point.y >= height * num_row || _point.y <= 0) return true;
	return false;
}

static bool is_a_wall(const Vector2 _point, const map_t _map)
{
	Vector2 local_point = get_ingame_coordinates(_point);

	if(is_outside_map(_point))
	{
		printf("hit wall 1\n");
		return true;
	}

	if(_point.x % width == 0)
	{
		printf("hit wall 2\n");
		if(_map[local_point.y][local_point.x] > 0 ||
				_map[local_point.y][local_point.x - 1] > 0)
			return true;
	}
	if(_point.y % height == 0)
	{
		printf("hit wall 3\n");
		if(_map[local_point.y][local_point.x] > 0 ||
				_map[local_point.y - 1][local_point.x] > 0)
			return true;
	}

	return false;
}

static void update_camera_direction(const SDL_MouseMotionEvent _e,
									player_t* _player_ctx)
{
	_player_ctx->camera_direction.x = _e.x;
	_player_ctx->camera_direction.y = _e.y;
}


//make it const
static void draw_square(SDL_Renderer** _renderer, int32_t col_num, int32_t row_num)
{
	SDL_FRect cell;
	cell.x = col_num * width;
	cell.y = row_num * height;
	cell.w = width;
	cell.h = height;

	SDL_SetRenderDrawColor(*_renderer, 210, 144, 114, 100);
	SDL_RenderFillRect(*_renderer, &cell);
	SDL_SetRenderDrawColor(*_renderer, 9, 100, 100, 100);

	return;
}

static void render_ray(SDL_Renderer** _renderer,
						const Vector2 _source,
						const Vector2 _direction)
{
	int32_t b_x, e_x, b_y, e_y;

	if(_source.x > _direction.x)
	{
		b_x = _direction.x;
		e_x = _source.x;
	}
	else
	{
		b_x = _source.x;
		e_x = _direction.x;
	}

	if(_source.y > _direction.y)
	{
		b_y = _direction.y;
		e_y = _source.y;
	}
	else
	{
		b_y = _source.y;
		e_y = _direction.y;
	}

	Vector2 closest;
	closest.x = 0;
	closest.y = 0;

	Vector2 current;
	current.x = 0;
	current.y = 0;

	bool found_target = false;
	//calculate linear equation for this sucker
	double a = _direction.y - _source.y;
	double b = _source.x - _direction.x;
	double c = a * (double)_source.x + b * (double)_source.y;

	SDL_SetRenderDrawColor(*_renderer, 144, 39, 89, 100);
	if(_source.x > _direction.x)
	{
		int32_t start = _source.x - (_source.x % width);
		for(int32_t j = start; j >= 0; j -= width)
		{
			current.x = (int32_t)j;
			//change this later so that it works before for
			if(b == 0) continue;
			current.y = (c - (a * (double)j))/b;

			if(!is_a_wall(current, g_map))
			{
				continue;
			}
			printf("1\n");
			found_target = true;
			closest = current;
			break;
		}
	}
	else
	{
		int32_t start = _source.x + (width - (_source.x % width));
		printf("starting value:%d\n", start);
		for(int32_t j = start; j <= width * num_col; j += width)
		{
			current.x = (int32_t)j;
			if(b == 0) continue;
			current.y = (c - (a * (double)j))/b;

			if(!is_a_wall(current, g_map))
			{
				continue;
			}
			printf("2\n");
			found_target = true;
			closest = current;
			break;
		}
	}

	a = _direction.x - _source.x;
	b = _source.y - _direction.y;
	c = a * (double)_source.y + b * (double)_source.x;

	if(_source.y > _direction.y)
	{
		int32_t start = _source.y - (_source.y % height);
		for(int32_t j = start; j >= 0; j -= height)
		{

			current.y = (int32_t)j;
			if(b == 0) continue;
			current.x = (c - (a * (double)j))/b;
			if(!is_a_wall(current, g_map))
			{
				continue;
			}
			printf("3\n");
			found_target = true;

			if(closest.x == 0 && closest.y == 0)
			{
				SDL_SetRenderDrawColor(*_renderer, 200, 144, 44, 100);
				closest = current;
				printf("3 - updating\n");
			}
			else
			{
				if(get_distance(_source, current) < get_distance(_source, closest))
				{
					SDL_SetRenderDrawColor(*_renderer, 200, 144, 44, 100);
					closest = current;
				printf("3 - updating\n");
				}
			}
			break;
		}
	}
	else
	{
		int32_t start = _source.y + (height - (_source.y % height));
		for(int32_t j = start; j <= height * num_row; j += height)
		{
			current.y = (int32_t)j;
			if(b == 0) continue;
			current.x = (c - (a * (double)j))/b;
			if(!is_a_wall(current, g_map))
			{
				continue;
			}
			printf("4\n");
			found_target = true;

			if(closest.x == 0 && closest.y == 0)
			{
				SDL_SetRenderDrawColor(*_renderer, 200, 144, 44, 100);
				closest = current;
			}
			else
			{
				if(get_distance(_source, current) < get_distance(_source, closest))
				{
					SDL_SetRenderDrawColor(*_renderer, 200, 144, 44, 100);
					closest = current;
				}
			}
			break;
		}
	}

	draw_circle(_renderer, (int32_t)closest.x, (int32_t)closest.y, 5, true);
	if(found_target)
		SDL_RenderLine(*_renderer, _source.x, _source.y, closest.x, closest.y);
	else
		SDL_RenderLine(*_renderer, _source.x, _source.y, _direction.x, _direction.y);
}

static void render_ray_d(SDL_Renderer** _renderer,
						const Vector2 _source,
						const Vector2_d _direction)
{
	int32_t b_x, e_x, b_y, e_y;

	if(_source.x > _direction.x)
	{
		b_x = _direction.x;
		e_x = _source.x;
	}
	else
	{
		b_x = _source.x;
		e_x = _direction.x;
	}

	if(_source.y > _direction.y)
	{
		b_y = _direction.y;
		e_y = _source.y;
	}
	else
	{
		b_y = _source.y;
		e_y = _direction.y;
	}

	Vector2 closest;
	closest.x = 0;
	closest.y = 0;

	Vector2 current;
	current.x = 0;
	current.y = 0;

	bool found_target = false;
	//calculate linear equation for this sucker
	double a = _direction.y - _source.y;
	double b = _source.x - _direction.x;
	double c = a * (double)_source.x + b * (double)_source.y;

	SDL_SetRenderDrawColor(*_renderer, 144, 39, 89, 100);
	if(_source.x > _direction.x)
	{
		int32_t start = _source.x - (_source.x % width);
		for(int32_t j = start; j >= 0; j -= width)
		{
			current.x = (int32_t)j;
			//change this later so that it works before for
			if(b == 0) continue;
			current.y = (c - (a * (double)j))/b;

			if(!is_a_wall(current, g_map))
			{
				continue;
			}
			printf("1\n");
			found_target = true;
			closest = current;
			break;
		}
	}
	else
	{
		int32_t start = _source.x + (width - (_source.x % width));
		printf("starting value:%d\n", start);
		for(int32_t j = start; j <= width * num_col; j += width)
		{
			current.x = (int32_t)j;
			if(b == 0) continue;
			current.y = (c - (a * (double)j))/b;

			if(!is_a_wall(current, g_map))
			{
				continue;
			}
			printf("2\n");
			found_target = true;
			closest = current;
			break;
		}
	}

	a = _direction.x - _source.x;
	b = _source.y - _direction.y;
	c = a * (double)_source.y + b * (double)_source.x;

	if(_source.y > _direction.y)
	{
		int32_t start = _source.y - (_source.y % height);
		for(int32_t j = start; j >= 0; j -= height)
		{

			current.y = (int32_t)j;
			if(b == 0) continue;
			current.x = (c - (a * (double)j))/b;
			if(!is_a_wall(current, g_map))
			{
				continue;
			}
			printf("3\n");
			found_target = true;

			if(closest.x == 0 && closest.y == 0)
			{
				SDL_SetRenderDrawColor(*_renderer, 200, 144, 44, 100);
				closest = current;
				printf("3 - updating\n");
			}
			else
			{
				if(get_distance(_source, current) < get_distance(_source, closest))
				{
					SDL_SetRenderDrawColor(*_renderer, 200, 144, 44, 100);
					closest = current;
				printf("3 - updating\n");
				}
			}
			break;
		}
	}
	else
	{
		int32_t start = _source.y + (height - (_source.y % height));
		for(int32_t j = start; j <= height * num_row; j += height)
		{
			current.y = (int32_t)j;
			if(b == 0) continue;
			current.x = (c - (a * (double)j))/b;
			if(!is_a_wall(current, g_map))
			{
				continue;
			}
			printf("4\n");
			found_target = true;

			if(closest.x == 0 && closest.y == 0)
			{
				SDL_SetRenderDrawColor(*_renderer, 200, 144, 44, 100);
				closest = current;
			}
			else
			{
				if(get_distance(_source, current) < get_distance(_source, closest))
				{
					SDL_SetRenderDrawColor(*_renderer, 200, 144, 44, 100);
					closest = current;
				}
			}
			break;
		}
	}

	draw_circle(_renderer, (int32_t)closest.x, (int32_t)closest.y, 5, true);
	if(found_target)
		SDL_RenderLine(*_renderer, _source.x, _source.y, closest.x, closest.y);
	else
		SDL_RenderLine(*_renderer, _source.x, _source.y, _direction.x, _direction.y);
}

static camera_plane_t render_camera_plane(SDL_Renderer** _renderer,
								const player_t _player_ctx)
{
	camera_plane_t ret_plane = (camera_plane_t)malloc(3 * sizeof(Vector2_d));

	const double distance = 100.0;

	Vector2_d versor;
	versor.x = (double)(_player_ctx.camera_direction.x - _player_ctx.x);
	versor.y = (double)(_player_ctx.camera_direction.y - _player_ctx.y);

	Vector2_d p1;
	p1.x = _player_ctx.camera_direction.x;
	p1.y = _player_ctx.camera_direction.y;

	Vector2_d p2;
	p2.x = _player_ctx.x;
	p2.y = _player_ctx.y;

	double temp_dist = get_distance_d(p1, p2);

	Vector2_d unit_vec;
	unit_vec.x = versor.x / temp_dist;
	unit_vec.y = versor.y / temp_dist;

	Vector2_d new_point;
	new_point.x = _player_ctx.x + unit_vec.x * distance;
	new_point.y = _player_ctx.y + unit_vec.y * distance;

	Vector2_d unit_vec2, temp2, unit_vec3;
	unit_vec2.x = unit_vec.x * -1;
	unit_vec2.y = unit_vec.y * -1;

	temp2 = unit_vec2;
	unit_vec2.x = temp2.y * -1;
	unit_vec2.y = temp2.x;

	unit_vec3.x = temp2.y;
	unit_vec3.y = temp2.x * -1;

	Vector2_d new_point2;
	new_point2.x = new_point.x + unit_vec2.x * distance/2;
	new_point2.y = new_point.y + unit_vec2.y * distance/2;

	Vector2_d new_point3;
	new_point3.x = new_point.x + unit_vec3.x * distance/2;
	new_point3.y = new_point.y + unit_vec3.y * distance/2;

	SDL_SetRenderDrawColor(*_renderer, 139, 11, 69, 100);
	draw_circle(_renderer, (int32_t)new_point.x, (int32_t)new_point.y, 5, true);
	draw_circle(_renderer, (int32_t)new_point2.x, (int32_t)new_point2.y, 5, true);
	draw_circle(_renderer, (int32_t)new_point3.x, (int32_t)new_point3.y, 5, true);
	SDL_RenderLine(*_renderer, new_point2.x, new_point2.y, new_point3.x, new_point3.y);
	*ret_plane = new_point;
	*(ret_plane + 1) = new_point2;
	*(ret_plane + 2) = new_point3;
	return ret_plane;
}

static void render_player(SDL_Renderer** _renderer, const player_t _player_ctx)
{
	SDL_SetRenderDrawColor(*_renderer, 244, 244, 0, 0);
	draw_circle(_renderer, (int32_t)_player_ctx.x, (int32_t)_player_ctx.y, 15, true);
	SDL_SetRenderDrawColor(*_renderer, 9, 100, 100, 100);
	Vector2 temp_pos = {.x = _player_ctx.x, .y = _player_ctx.y};
	render_ray(_renderer, temp_pos, _player_ctx.camera_direction);
	//render camera plane
	camera_plane_t test = render_camera_plane(_renderer, _player_ctx);

	for(int i = 0; i < 3; ++i)
	{
		render_ray_d(_renderer, temp_pos, *(test + i));
	}

	//does this work this way?
	free(test);
}

static bool key_pressed(SDL_KeyboardEvent _e, direction_t* _dir,
						player_t* _player_ctx)
{
	*_dir = STATIC;

	//leave this for now - in future i will be doing some
	//more advanced controls here
	switch(_e.key)
	{
		case SDLK_W:
			*_dir = UP;
			_player_ctx->y -= 10;
			break;
		case SDLK_S:
			*_dir = DOWN;
			_player_ctx->y += 10;
			break;
		case SDLK_A:
			*_dir = LEFT;
			_player_ctx->x -= 10;
			break;
		case SDLK_D:
			*_dir = RIGHT;
			_player_ctx->x += 10;
			break;
		case SDLK_ESCAPE:
			return 1;
	}


	return 0;
}

static void mouse_callback(SDL_MouseButtonEvent _e)
{
	starting_point.x = (int32_t)_e.x;
	starting_point.y = (int32_t)_e.y;

	for(int i = 0; i < 100; ++i)
	{
		if(coords[i].x == 0 && coords[i].y == 0)
		{
			coords[i].x = (int32_t)_e.x;
			coords[i].y = (int32_t)_e.y;
			return;
		}
	}
	printf("circle limit reached\n");
}

static void draw_map_and_grid(SDL_Renderer* _renderer, const map_t _map,
						int32_t num_col, int32_t num_row)
{
	SDL_SetRenderDrawColor(_renderer, 9, 50, 50, 50);

	for(int i = 0; i <= num_col; ++i)
	{
		SDL_RenderLine(_renderer, i * width, 0, i * width, width * num_row);
	}
	for(int i = 0; i <= num_row; ++i)
	{
		SDL_RenderLine(_renderer, 0, i * height, height * num_col, i * height);
	}

	for(int i = 0; i < num_col; ++i)
	{
		for(int j = 0; j < num_row; ++j)
		{
			if(_map[i][j] > 0) draw_square(&_renderer, j, i);
		}
	}
}

void run_game()
{
	SDL_Init(SDL_INIT_VIDEO);

	struct SDL_Window* wind;
	struct SDL_Renderer* rend;

	player.x = WIDTH/2;
	player.y = HEIGHT/2;
	memset(coords, 0, 100);

	SDL_CreateWindowAndRenderer("DO_NOT_RESIZE", HEIGHT, WIDTH,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE , &wind, &rend);

	bool quit = false;
	while (!quit)
	{
		quit = update_and_render(&rend, &wind);
	}


	// free
	if (rend != NULL)
	{
		SDL_DestroyRenderer(rend);
		rend = NULL;
	}
	if (wind!= NULL)
	{
		SDL_DestroyWindow(wind);
		wind = NULL;
	}
	SDL_Quit();
}

