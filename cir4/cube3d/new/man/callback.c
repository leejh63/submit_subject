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
#define KEY_LEFT	0xff51
#define KEY_RIGHT	0xff53
#define KEY_W		0x0077
#define KEY_A		0x0061
#define KEY_S		0x0073
#define KEY_D		0x0064
#define KEY_SPACE	0x0020

int	presskey_callback(int key, void *param)
{
	t_param	*p;

	p = (t_param *)param;
	printf("key : %x\n", key);
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
	else if (key == KEY_LEFT)
		p->key[LEFT] = 1;
	else if (key == KEY_RIGHT)
		p->key[RIGHT] = 1;
	else if (key == KEY_SPACE)
		p->key[SPACE] = 1;
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
	const double	lrdeg = 0.032;
	t_param			*p;

	p = (t_param *)param;
	if (p->v.pre_mouse_x == x && p->v.pre_mouse_y == y)
		return (0);
	if (p->v.pre_mouse_x > x)
	{
		p->v.theta -= lrdeg;
		if (p->v.theta < 0)
			p->v.theta += M_PI * 2;
	}
	if (p->v.pre_mouse_x < x)
	{
		p->v.theta += lrdeg;
		if (p->v.theta >= M_PI * 2)
			p->v.theta -= M_PI * 2;
	}
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
	mlx_put_image_to_window(p->mlx, p->win, p->img, 0, 0);
	return (0);
}
