#ifndef GAME_H
#define GAME_H

# define WIDTH 1280
# define HEIGHT 720

#include "./mlx/mlx.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

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

#endif
