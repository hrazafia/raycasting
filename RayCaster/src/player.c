#include "../includes/game.h"

void	init_player(t_player *player)
{
	player->x = WIDTH / 2;
	player->y = HEIGHT / 2;
	player->key_up = false;
	player->key_down = false;
	player->key_right = false;
	player->key_left = false;
}

int	key_release(int keycode, t_player *player)
{
	if (keycode == W)
		player->key_up = true;
	if (keycode = S)
		player->key_down = true;
	if (keycode = A)
		player->key_left = true;
	if (keycode = D)
		player->key_right = true;
	return 0;
}

int	key_press(int keycode, t_player *player)
{
	if (keycode == W)
		player->key_up = false;
	if (keycode = S)
		player->key_down = false;
	if (keycode = A)
		player->key_left = false;
	if (keycode = D)
		player->key_right = false;
	return 0;
}
