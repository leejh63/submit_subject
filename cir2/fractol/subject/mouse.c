/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mouse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/10 10:56:20 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/10 22:12:27 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	mouse_input(int button, int x, int y, t_ap *ap)
{
	if (button == 2)
		move_mouse(x, y, ap);
	if (button == 4)
		zoom_in(x, y, ap);
	if (button == 5)
		zoom_out(x, y, ap);
	return (0);
}

void	move_mouse(int x, int y, t_ap *ap)
{
	void	*mlx;
	void	*win;
	void	*img;

	mlx = (*ap).mlx->mlx;
	win = (*ap).mlx->win;
	img = (*ap).mlx->img;
	(*ap).num->str_x = convert_x(x, (*ap).num);
	(*ap).num->str_y = convert_y(y, (*ap).num);
	fractal_check_dot((*ap).mlx, (*ap).num);
	mlx_put_image_to_window(mlx, win, img, 0, 0);
}

void	zoom_in(int x, int y, t_ap *ap)
{
	void	*mlx;
	void	*win;
	void	*img;

	mlx = (*ap).mlx->mlx;
	win = (*ap).mlx->win;
	img = (*ap).mlx->img;
	(*ap).num->scale = 0.96;
	(*ap).num->str_x = convert_x(x, (*ap).num);
	(*ap).num->str_y = convert_y(y, (*ap).num);
	(*ap).num->max_x *= (*ap).num->scale;
	(*ap).num->max_y *= (*ap).num->scale;
	(*ap).num->min_x *= (*ap).num->scale;
	(*ap).num->min_y *= (*ap).num->scale;
	(*ap).num->max_repeat += 10;
	if ((*ap).num->max_repeat > 300)
		(*ap).num->max_repeat = 300;
	fractal_check_dot((*ap).mlx, (*ap).num);
	mlx_put_image_to_window(mlx, win, img, 0, 0);
}

void	zoom_out(int x, int y, t_ap *ap)
{
	void	*mlx;
	void	*win;
	void	*img;

	mlx = (*ap).mlx->mlx;
	win = (*ap).mlx->win;
	img = (*ap).mlx->img;
	(*ap).num->scale = 0.96;
	(*ap).num->str_x = convert_x(x, (*ap).num);
	(*ap).num->str_y = convert_y(y, (*ap).num);
	(*ap).num->max_x /= (*ap).num->scale;
	(*ap).num->max_y /= (*ap).num->scale;
	(*ap).num->min_x /= (*ap).num->scale;
	(*ap).num->min_y /= (*ap).num->scale;
	(*ap).num->max_repeat -= 10;
	if ((*ap).num->max_repeat < 100)
		(*ap).num->max_repeat = 100;
	fractal_check_dot((*ap).mlx, (*ap).num);
	mlx_put_image_to_window(mlx, win, img, 0, 0);
}
