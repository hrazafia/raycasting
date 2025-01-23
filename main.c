/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hrazafia <hrazafia@student.42antanana      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/22 07:40:02 by hrazafia          #+#    #+#             */
/*   Updated: 2025/01/23 08:49:49 by hrazafia         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <X11/X.h>
#include <X11/keysym.h>
#include <mlx.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define MAP_WIDTH 15
#define MAP_HEIGHT 15
#define TEX_WIDTH 64
#define TEX_HEIGHT 64
#define MOVE_SPEED 2
#define ROT_SPEED 1
#define COLLISION_MARGIN 0.2
#define COLLISION_DIST 0.4
#define FOV 66

#define WALL 1
#define INF -1

unsigned long	last_frame_time = 0;
double			delta_time = 0;
int				fps = 60;

typedef struct s_vector2d
{
	double	x;
	double	y;
}				t_vector2d;

typedef struct s_position
{
	int	x;
	int	y;
}				t_position;

typedef struct s_player
{
	t_vector2d	pos;
	t_vector2d	dir;
}				t_player;

typedef struct s_camera
{
	double		x;
	t_vector2d	plane;
}				t_camera;

typedef struct s_input
{
	int	left_rotate;
	int	right_rotate;
	int	key_up;
	int	key_down;
	int	key_left;
	int	key_right;
}				t_input;

typedef struct s_ray
{
	t_vector2d	dir;
	t_vector2d	pos;
	t_vector2d	side_dist;
	t_vector2d	delta_dist;
	int			side;
	double		perp_dist;
	t_position	step;
	t_position	map;
}				t_ray;

typedef struct s_texture
{
	void	*img;
	char	*addr;
	int		bpp;
	int		line_len;
	int		endian;
}				t_texture;

typedef struct s_data
{
	void		*mlx;
	void		*win;
	void		*img;
	char		*addr;
	int			bpp;
	int			line_len;
	int			endian;
	t_player	player;
	t_camera	camera;
	t_input		input;
	int			map[MAP_WIDTH][MAP_HEIGHT];
	t_texture	textures[4];
}				t_data;

typedef void	(*t_func)(t_data *data, t_ray *ray, int x);

unsigned long	get_time_in_ms()
{
	struct timeval	time;
	gettimeofday(&time, NULL);
	return (time.tv_sec * 1000) + (time.tv_usec / 1000);
}

void	draw_pixel(t_data *data, int x, int y, int color)
{
	if (x >= 0 && x < WIN_WIDTH && y >= 0 && y < WIN_HEIGHT)
	{
		char *pixel = data->addr + (y * data->line_len + x * (data->bpp / 8));
		*(unsigned int *)pixel = color;
	}
}

void	draw_ceil_floor(t_data *data)
{
	int rows;
	int cols;

	rows = 0;
	while (rows < WIN_HEIGHT / 2)
	{
		cols = 0;
		while (cols < WIN_WIDTH)
		{
			draw_pixel(data, cols, rows, 0x59514A);
			cols++;
		}
		rows++;
	}
	while (rows < WIN_HEIGHT)
	{
		cols = 0;
		while (cols < WIN_WIDTH)
		{
			draw_pixel(data, cols, rows, 0xA0522D);
			cols++;
		}
		rows++;
	}
}

unsigned int	get_texture_pixel(t_texture *texture, int x, int y)
{
	char *pixel = texture->addr + (y * texture->line_len + x * (texture->bpp / 8));
	return *(unsigned int *) pixel;
}

void load_texture(t_data *data, int index, char *path)
{
	data->textures[index].img = mlx_xpm_file_to_image(data->mlx, path, &(int){TEX_WIDTH}, &(int){TEX_HEIGHT});
	if (!data->textures[index].img)
	{
		fprintf(stderr, "Erreur lors du chargement de la texture : %s\n", path);
		exit(EXIT_FAILURE);
	}
	data->textures[index].addr = mlx_get_data_addr(data->textures[index].img,
                                                   &data->textures[index].bpp,
                                                   &data->textures[index].line_len,
                                                   &data->textures[index].endian);
}

int	key_press(int keycode, t_data *data)
{
	if (keycode == XK_w)
		data->input.key_up = 1;
	if (keycode == XK_s)
		data->input.key_down = 1;
	if (keycode == XK_a)
		data->input.key_left = 1;
	if (keycode == XK_d)
		data->input.key_right = 1;
	if (keycode == XK_Left)
		data->input.left_rotate = 1;
	if (keycode == XK_Right)
		data->input.right_rotate = 1;
	if (keycode == XK_Escape)
		exit(EXIT_SUCCESS);
	return 0;
}

int	key_release(int keycode, t_data *data)
{
	if (keycode == XK_w)
		data->input.key_up = 0;
	if (keycode == XK_s)
		data->input.key_down = 0;
	if (keycode == XK_a)
		data->input.key_left = 0;
	if (keycode == XK_d)
		data->input.key_right = 0;
	if (keycode == XK_Left)
		data->input.left_rotate = 0;
    if (keycode == XK_Right)
		data->input.right_rotate = 0;
	return 0;
}

int	collision(t_data *data, double x, double y, int entity)
{
	double	dist;

	dist = COLLISION_MARGIN;
	return (data->map[(int)(x - dist)][(int)(y - dist)] == entity
			|| data->map[(int)(x + dist)][(int)(y - dist)] == entity
			|| data->map[(int)(x - dist)][(int)(y + dist)] == entity
			|| data->map[(int)(x + dist)][(int)(y + dist)] == entity);
}

void	translate(t_data *data, double move_x, double move_y)
{
	if (!collision(data, data->player.pos.x + move_x, data->player.pos.y, WALL))
		data->player.pos.x += move_x;
	if (!collision(data, data->player.pos.x, data->player.pos.y + move_y, WALL))
		data->player.pos.y += move_y;
}

void	translate_forward(t_data *data, t_player *player)
{
	double move_x;
	double move_y;

	move_x = data->player.dir.x * MOVE_SPEED * delta_time;
	move_y = data->player.dir.y * MOVE_SPEED * delta_time;
	translate(data, move_x, move_y);
}

void	translate_backward(t_data *data, t_player *player)
{
	double move_x;
	double move_y;

	move_x = -data->player.dir.x * MOVE_SPEED * delta_time;
	move_y = -data->player.dir.y * MOVE_SPEED * delta_time;
	translate(data, move_x, move_y);
}

void	translate_left(t_data *data, t_player *player)
{
	double move_x;
	double move_y;

	move_x = -data->camera.plane.x * MOVE_SPEED * delta_time;
	move_y = -data->camera.plane.y * MOVE_SPEED * delta_time;
	translate(data, move_x, move_y);
}

void	translate_right(t_data *data, t_player *player)
{
	double move_x;
	double move_y;

	move_x = data->camera.plane.x * MOVE_SPEED * delta_time;
	move_y = data->camera.plane.y * MOVE_SPEED * delta_time;
	translate(data, move_x, move_y);
}

void	rotate_left(t_data *data, t_player *player)
{
	double old_dir_x;
	double old_plane_x;
	
	old_dir_x = data->player.dir.x;
	data->player.dir.x = data->player.dir.x * cos(ROT_SPEED * delta_time)
		- data->player.dir.y * sin(ROT_SPEED * delta_time);
	data->player.dir.y = old_dir_x * sin(ROT_SPEED * delta_time)
		+ data->player.dir.y * cos(ROT_SPEED * delta_time);
	old_plane_x = data->camera.plane.x;
	data->camera.plane.x = data->camera.plane.x * cos(ROT_SPEED * delta_time)
		- data->camera.plane.y * sin(ROT_SPEED * delta_time);
	data->camera.plane.y = old_plane_x * sin(ROT_SPEED * delta_time)
		+ data->camera.plane.y * cos(ROT_SPEED * delta_time);
}

void	rotate_right(t_data *data, t_player *player)
{
	double old_dir_x;
	double old_plane_x;
	
	old_dir_x = data->player.dir.x;
	data->player.dir.x = data->player.dir.x * cos(-ROT_SPEED * delta_time)
		- data->player.dir.y * sin(-ROT_SPEED * delta_time);
	data->player.dir.y = old_dir_x * sin(-ROT_SPEED * delta_time)
		+ data->player.dir.y * cos(-ROT_SPEED * delta_time);
	old_plane_x = data->camera.plane.x;
	data->camera.plane.x = data->camera.plane.x * cos(-ROT_SPEED * delta_time)
		- data->camera.plane.y * sin(-ROT_SPEED * delta_time);
	data->camera.plane.y = old_plane_x * sin(-ROT_SPEED * delta_time)
		+ data->camera.plane.y * cos(-ROT_SPEED * delta_time);
}

int handle_key(t_data *data)
{
	if (data->input.key_up)
		translate_forward(data, &data->player);
	if (data->input.key_down)
		translate_backward(data, &data->player);
	if (data->input.key_left)
		translate_left(data, &data->player);
	if (data->input.key_right)
		translate_right(data, &data->player);
	if (data->input.left_rotate)
		rotate_left(data, &data->player);
	if (data->input.right_rotate)
		rotate_right(data, &data->player);
	return 0;
}

void	init_ray(t_data *data, t_ray *ray, int x)
{
	data->camera.x = 2 * x / (double) WIN_WIDTH - 1;
	ray->dir.x = data->player.dir.x + data->camera.plane.x * data->camera.x;
	ray->dir.y = data->player.dir.y + data->camera.plane.y * data->camera.x;

	ray->map.x = (int) data->player.pos.x;
	ray->map.y = (int) data->player.pos.y;

	ray->delta_dist.x = (ray->dir.x == 0) ? 1e30 : fabs(1 / ray->dir.x);
	ray->delta_dist.y = (ray->dir.y == 0) ? 1e30 : fabs(1 / ray->dir.y);

	if (ray->dir.x < 0)
	{
		ray->step.x = -1;
		ray->side_dist.x = (data->player.pos.x - ray->map.x) * ray->delta_dist.x;
	}
	else
	{
		ray->step.x = 1;
		ray->side_dist.x = (ray->map.x + 1.0 - data->player.pos.x) * ray->delta_dist.x;
	}
	if (ray->dir.y < 0)
	{
		ray->step.y = -1;
		ray->side_dist.y = (data->player.pos.y - ray->map.y) * ray->delta_dist.y;
	}
	else
	{
		ray->step.y = 1;
		ray->side_dist.y = (ray->map.y + 1.0 - data->player.pos.y) * ray->delta_dist.y;
	}
}

int	is_limit(int step, int limit)
{
	if (limit == -1)
		return (0);
	if (step < limit)
		return (0);
	else
		return (1);
}

void	dda(t_data *data, t_ray *ray, int limit)
{
	int	step;
	int	hit;

	hit = 0;
	step = 0;
	while (!hit && !is_limit(step, limit))
	{
		if (ray->side_dist.x < ray->side_dist.y)
		{
			ray->side_dist.x += ray->delta_dist.x;
			ray->map.x += ray->step.x;
			ray->side = 0;
		}
		else
		{
			ray->side_dist.y += ray->delta_dist.y;
			ray->map.y += ray->step.y;
			ray->side = 1;
		}
		if (data->map[ray->map.x][ray->map.y] > 0)
			hit = 1;
		step++;
	}
}

void	calc_perp_dist(t_data *data, t_ray *ray)
{
	if (ray->side == 0)
	{
		ray->perp_dist = (ray->map.x - data->player.pos.x
			+ (1 - ray->step.x) / 2) / ray->dir.x;
	}
	else
	{
		ray->perp_dist = (ray->map.y - data->player.pos.y
			+ (1 - ray->step.y) / 2) / ray->dir.y;
	}

}

void	draw_wall(t_data *data, t_ray *ray, int x)
{
	int lineHeight = (int)(WIN_HEIGHT / ray->perp_dist);

	int drawStart = -lineHeight / 2 + WIN_HEIGHT / 2;
	if (drawStart < 10) drawStart = 0;
	int drawEnd = lineHeight / 2 + WIN_HEIGHT / 2;
	if (drawEnd >= WIN_HEIGHT) drawEnd = WIN_HEIGHT - 1;

	int texNum = 0;
	if (ray->side == 1)
		texNum = (ray->step.y > 0) ? 2 : 3;
	else
		texNum = (ray->step.x > 0) ? 1 : 0;
		
	double wallX;
	if (ray->side == 0)
		wallX = data->player.pos.y + ray->perp_dist * ray->dir.y;
	else
		wallX = data->player.pos.x + ray->perp_dist * ray->dir.x;

		wallX -= floor(wallX);

	int texX = (int)(wallX * TEX_WIDTH);
		
	if (ray->side == 0 && ray->dir.x > 0) texX = TEX_WIDTH - texX - 1;
	if (ray->side == 1 && ray->dir.y < 0) texX = TEX_WIDTH - texX - 1;

	for (int y = drawStart; y < drawEnd; y++)
	{
		int d = y * 256 - WIN_HEIGHT * 128 + lineHeight * 128;
		int texY = ((d * TEX_HEIGHT) / lineHeight) / 256;
		unsigned int color = get_texture_pixel(&data->textures[texNum], texX, texY);
		draw_pixel(data, x, y, color);
	}
}

void	raycasting(t_data *data, int limit, t_func func)
{
	int	x;
	t_ray	ray;
	
	x = 0;
	while (x < WIN_WIDTH)
	{
		init_ray(data, &ray, x);
		dda(data, &ray, limit);
		calc_perp_dist(data, &ray);
		func(data, &ray, x);
		x++;
	}
}

int	render(t_data *data) 
{
	unsigned long	current_time = get_time_in_ms();
	unsigned long	frame_time = current_time - last_frame_time;

	if (frame_time < (1000 / fps))
		return (0);
	last_frame_time = current_time;
	delta_time = frame_time / 1000.0;
	handle_key(data);
	draw_ceil_floor(data);
	raycasting(data, INF, draw_wall);
	mlx_put_image_to_window(data->mlx, data->win, data->img, 0, 0);
	return (0);
}

int	camera_plane(t_data *data, double dir_x, double dir_y, double fov)
{
	double	plane_scale;

   	if ((fov <= 0) || (fov >= 180) || ((dir_x == 0) && (dir_y == 0)))
		return (-1);
	plane_scale = tan((fov / 2.0) * (M_PI / 180.0));
	data->camera.plane.y = -dir_x * plane_scale;
	data->camera.plane.x = dir_y * plane_scale;
	return (0);
}

int main()
{
	char	direction;
	t_data	data;

	data.mlx = mlx_init();
	data.win = mlx_new_window(data.mlx, WIN_WIDTH, WIN_HEIGHT, "Cub3D");
	data.img = mlx_new_image(data.mlx, WIN_WIDTH, WIN_HEIGHT);
	data.addr = mlx_get_data_addr(data.img, &data.bpp, &data.line_len, &data.endian);

	data.player.pos.x = 7;
	data.player.pos.y = 7;

	direction = 'N';
    
	if (direction == 'N')
	{
		data.player.dir.x = -1;
		data.player.dir.y = 0;
	}
	else if (direction == 'S')
	{
		data.player.dir.x = 1;
		data.player.dir.y = 0;
	}
	else if (direction == 'E')
	{
		data.player.dir.x = 0;
		data.player.dir.y = 1;
	}
	else if (direction == 'W')
	{
		data.player.dir.x = 0;
		data.player.dir.y = -1;
	}

	camera_plane(&data, data.player.dir.x, data.player.dir.y, FOV);
    
	data.input.key_up = 0;
	data.input.key_down = 0;
	data.input.key_left = 0;
	data.input.key_right = 0;

	data.input.left_rotate = 0;
	data.input.right_rotate = 0;

	int map[MAP_WIDTH][MAP_HEIGHT] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
        {1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1},
        {1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

	for (int i = 0; i < MAP_WIDTH; i++)
		for (int j = 0; j < MAP_HEIGHT; j++)
			data.map[i][j] = map[i][j];

	load_texture(&data, 0, "nord.xpm");
	load_texture(&data, 1, "sud.xpm");
	load_texture(&data, 2, "est.xpm");
	load_texture(&data, 3, "ouest.xpm");

	mlx_hook(data.win, 2, 1L << 0, key_press, &data);
	mlx_hook(data.win, 3, 1L << 1, key_release, &data);
	mlx_loop_hook(data.mlx, render, &data);
	mlx_loop(data.mlx);
    return 0;
}
