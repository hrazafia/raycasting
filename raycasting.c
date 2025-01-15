typedef struct s_player
{
	double	pos_x;
	double	pos_y;
	double	dir_x;
	double	dir_y;
}		t_player;

typedef struct s_camera
{
	double	plane_x;
	double	plane_y;
	double	x;
}		t_camera;

typedef struct s_input
{
	int	left_rotate;
	int	right_rotate;
	int	key_up;
	int	key_down;
	int	key_left;
	int	key_right;
}		t_input;

typedef struct s_ray_state
{
	double	side_dist_x;
	double	side_dist_y;
	double	delta_dist_x;
	double	delta_dist_y;
	int	step_x;
	int	step_y;
	int	map_x;
	int	map_y;
	int	side;
	double	perp_dist;
}		t_ray_state;


typedef struct s_ray
{
	double		dir_x;
	double		dir_y;
	double		pos_x;
	double		pos_y;
	t_ray_state	state;
}		t_ray;

int	x;
t_ray	ray;
int	hit;

x = 0;
while (x < WIN_WIDTH)
{
	data->camera.x = 2 * x / (double) WIN_WIDTH - 1;
        ray.dir_x = data->player.dir_x + camera.plane_x * camera.x;
        ray.dir_y = data->player.dir_y + camera.plane_y * camera.x;
	ray.state.map_x = data->player.pos_x;
	ray.state.map_y = data->player.pos_y;
	
	ray.state.delta_dist_x = fabs(1 / ray.dir_x);
	ray.state.delta_dist_y = fabs(1 / ray.dir_y);

	if (ray.dir_x < 0)
        {
		ray.state.step_x = -1;
		ray.state.side_dist_x = (data->player.pos_x - ray.state.map_x) * ray.state.delta_dist_x;
        }
        else
        {
            ray.state.step_x = 1;
            ray.state.side_dist_x = (ray.state.map_x + 1.0 - data->player.pos_x) * ray.state.delta_dist_x;
        }
        if (ray.dir_y < 0)
        {
		ray.state.step_y = -1;
		ray.state.side_dist_y = (data->player.pos_y - ray.state.map_y) * ray.state.delta_dist_y;
        }
        else
        {
            ray.state.step_y = 1;
            ray.state_side_dist_y = (ray.state.map_y + 1.0 - data->player.pos_y) * ray.state.delta_dist_y;
        }
        
        
	hit = 0;
	while (!hit)
	{
		if (ray.state.side_dist_x < ray.state_side_dist_y)
		{
			ray.state.side_dist_x += ray.state.delta_dist_x
			ray.state.map_x += ray.state.step_x;
			ray.state.side = 0;
		}
		else
		{
			ray.state.side_dist_y += ray.state.delta_dist_y;
			ray.state.map_y += ray.state.step_y;
			ray.state.side = 1;
		}
		if (data->map[ray.state.map_x][ray.state.map_y] == 1
			|| data->map[ray.state.map_x][ray.state.map_y] == 2)
			hit = 1;
	}

        if (ray.state.side == 0)
            ray.state.perp_dist = (ray.state.map_x - data->player.pos_x + (1 - ray.state.step_x) / 2) / ray.dir_x;
        else
            ray.state.perp_dist = (ray.state.map_y - data->player.pos_y + (1 - ray.state.step_y) / 2) / ray.dir_y;

        z_buffer[x] = ray.state.perp_dist;

        int lineHeight = (int)(WIN_HEIGHT / perpWallDist);

        int drawStart = -lineHeight / 2 + WIN_HEIGHT / 2;
        if (drawStart < 0) drawStart = 0;
        int drawEnd = lineHeight / 2 + WIN_HEIGHT / 2;
        if (drawEnd >= WIN_HEIGHT) drawEnd = WIN_HEIGHT - 1;

        int texNum = 0;
        if (side == 1)
        {
            texNum = (stepY > 0) ? 0 : 1; // Rouge pour le mur nord, vert pour le mur sud
        }
        else
        {
            texNum = (stepX > 0) ? 2 : 3; // Bleu pour le mur est, jaune pour le mur ouest
        }

        if (data->map[mapX][mapY] == 2)
            texNum = 4;

        double wallX;
        if (side == 0)
            wallX = data->player.posY + perpWallDist * rayDirY;
        else
            wallX = data->player.posX + perpWallDist * rayDirX;
        wallX -= floor(wallX);

        int texX = (int)(wallX * TEX_WIDTH);
        if (side == 0 && rayDirX > 0) texX = TEX_WIDTH - texX - 1;
        if (side == 1 && rayDirY < 0) texX = TEX_WIDTH - texX - 1;

        for (int y = drawStart; y < drawEnd; y++)
        {
            int d = y * 256 - WIN_HEIGHT * 128 + lineHeight * 128;
            int texY = ((d * TEX_HEIGHT) / lineHeight) / 256;
            unsigned int color = get_texture_pixel(&data->textures[texNum], texX, texY);
            draw_pixel(data, x, y, color);
        }
	x++;
}
