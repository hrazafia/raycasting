#ifndef GAME_H
#define GAME_H

# define WIDTH 1280
# define HEIGHT 720

#include "./mlx/mlx.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

# define W 119
# define A 97
# define S 115
# define D 100 

# define PI 3.1459265359

typedef struct s_game
{
	void	*mlx;
	void	*win;
	void	*img;
	char	*data;
	int	bpp;
	int	size_line;
	int	endian;
}				t_game;

typedef struct s_player
{
	float x;
	float y;
	bool key_up;
	bool key_down;
	bool key_left;
	bool key_right;
}				t_player;

void	init_player(t_player *player);

#endif
