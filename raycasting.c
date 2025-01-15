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

typedef struct s_ray
{
	double		dir_x;
	double		dir_y;
	double		pos_x;
	double		pos_y;
	t_ray_state	state;
} t_ray;

for (int x = 0; x < WIN_WIDTH; x++)
{
        double cameraX = 2 * x / (double)WIN_WIDTH - 1;
        double rayDirX = data->player.dirX + data->player.planeX * cameraX;
        double rayDirY = data->player.dirY + data->player.planeY * cameraX;

        int mapX = (int)data->player.posX;
        int mapY = (int)data->player.posY;

        double sideDistX;
        double sideDistY;

        double deltaDistX = fabs(1 / rayDirX);
        double deltaDistY = fabs(1 / rayDirY);
        double perpWallDist;

        int stepX;
        int stepY;

        int hit = 0;
        int side;

        if (rayDirX < 0)
        {
            stepX = -1;
            sideDistX = (data->player.posX - mapX) * deltaDistX;
        }
        else
        {
            stepX = 1;
            sideDistX = (mapX + 1.0 - data->player.posX) * deltaDistX;
        }
        if (rayDirY < 0)
        {
            stepY = -1;
            sideDistY = (data->player.posY - mapY) * deltaDistY;
        }
        else
        {
            stepY = 1;
            sideDistY = (mapY + 1.0 - data->player.posY) * deltaDistY;
        }

        while (!hit)
        {
            if (sideDistX < sideDistY)
            {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            }
            else
            {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
            if (data->map[mapX][mapY] == 1 || data->map[mapX][mapY] == 2) hit = 1;
        }

        if (side == 0)
            perpWallDist = (mapX - data->player.posX + (1 - stepX) / 2) / rayDirX;
        else
            perpWallDist = (mapY - data->player.posY + (1 - stepY) / 2) / rayDirY;

        zBuffer[x] = perpWallDist; // Stocke la distance du mur pour c rayon.

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
    }
