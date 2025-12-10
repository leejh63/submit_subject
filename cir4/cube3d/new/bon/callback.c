/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   callback.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/26 14:53:43 by kkeum             #+#    #+#             */
/*   Updated: 2025/08/26 11:42:41 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

#define KEY_ESC		0xff1b
#define KEY_UP		0xff52
#define KEY_DOWN	0xff54
#define KEY_LEFT	0xff51
#define KEY_RIGHT	0xff53
#define KEY_W		0x0077
#define KEY_A		0x0061
#define KEY_S		0x0073
#define KEY_D		0x0064
#define KEY_SPACE	0x0020
#define KEY_CTRL	0xffe3

int	presskey_callback(int key, void *param)
{
	t_param	*p;

	p = (t_param *)param;
	if (key == KEY_ESC)
		mlx_loop_end(p->mlx);
	else if (key == KEY_W)
		p->key[W] = 1;
	else if (key == KEY_A)
		p->key[A] = 1;
	else if (key == KEY_S)
		p->key[S] = 1;
	else if (key == KEY_D)
		p->key[D] = 1;
	else if (key == KEY_UP)
		p->key[UP] = 1;
	else if (key == KEY_DOWN)
		p->key[DOWN] = 1;
	else if (key == KEY_LEFT)
		p->key[LEFT] = 1;
	else if (key == KEY_RIGHT)
		p->key[RIGHT] = 1;
	else if (key == KEY_SPACE)
		p->key[SPACE] = 1;
	else if (key == KEY_CTRL)
		p->key[CTRL] = 1;
	return (0);
}

int	releasekey_callback(int key, void *param)
{
	t_param	*p;

	p = (t_param *)param;
	if (key == KEY_ESC)
		mlx_loop_end(p->mlx);
	else if (key == KEY_W)
		p->key[W] = 0;
	else if (key == KEY_A)
		p->key[A] = 0;
	else if (key == KEY_S)
		p->key[S] = 0;
	else if (key == KEY_D)
		p->key[D] = 0;
	else if (key == KEY_UP)
		p->key[UP] = 0;
	else if (key == KEY_DOWN)
		p->key[DOWN] = 0;
	else if (key == KEY_LEFT)
		p->key[LEFT] = 0;
	else if (key == KEY_RIGHT)
		p->key[RIGHT] = 0;
	return (0);
}

int	close_win(void *param)
{
	(void)param;
	printf("Closing window...\n");
	exit(0);
}

int	mouse_callback(int x, int y, void *param)
{
	const double	deg[2] = {0.025, 0.045};
	t_param			*p;

	p = (t_param *)param;
	if (x == WIN_W / 2 && y == WIN_H / 2)
		return (0);
	if (p->v.pre_mouse_x > x)
		p->v.theta -= deg[1];
	if (p->v.pre_mouse_x < x)
		p->v.theta += deg[1];
	p->v.theta = fmod(p->v.theta, M_PI * 2);
	if (p->v.pre_mouse_y < y)
		if (p->v.verti > 0)
			p->v.verti -= deg[0];
	if (p->v.pre_mouse_y > y)
		if (p->v.verti < M_PI_4 + M_PI / 4)
			p->v.verti += deg[0];
	mlx_mouse_move(p->mlx, p->win, WIN_W / 2, WIN_H / 2);
	return (0);
}

int	ft_draw_screen(void *param)
{
	t_param	*p;
	double	x_y[5];
	double	plain[3];
	int		pix;

	p = (t_param *)param;
	move(p);
	p->dda_count = 0;
	x_y[0] = cos(p->v.theta);
	x_y[1] = sin(p->v.theta);
	x_y[2] = tan(M_PI_2 / 2.0);
	plain[0] = -x_y[1] * x_y[2];
	plain[1] = x_y[0] * x_y[2];
	pix = -1;
	while (++pix < WIN_W)
	{
		plain[2] = 2 * (pix / (double)WIN_W) - 1;
		x_y[3] = x_y[0] + plain[0] * plain[2];
		x_y[4] = x_y[1] + plain[1] * plain[2];
		dda_ray_cast(x_y[3], x_y[4], p, pix);
	}
	minimap_centered(p, 10);
	draw_animation(p);
	draw_fire(p);
	mlx_put_image_to_window(p->mlx, p->win, p->img, 0, 0);
	return (0);
}
