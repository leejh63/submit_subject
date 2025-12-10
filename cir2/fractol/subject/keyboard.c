/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   keyboard.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/10 11:14:02 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/10 22:13:48 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	key_input(int keycode, t_ap *ap)
{
	if (keycode == 65307)
		return (end_program(ap), 0);
	if (keycode >= 65361 && keycode <= 65364)
		key_move(keycode, ap);
	return (0);
}

void	key_move(int keycode, t_ap *ap)
{
	double	move_x;
	double	move_y;
	void	*mlx;
	void	*win;
	void	*img;

	mlx = (*ap).mlx->mlx;
	win = (*ap).mlx->win;
	img = (*ap).mlx->img;
	move_x = ((*ap).num->max_x - (*ap).num->min_x) / 5;
	move_y = ((*ap).num->max_y - (*ap).num->min_y) / 5;
	if (keycode == 65361)
		(*ap).num->str_x -= move_x;
	if (keycode == 65363)
		(*ap).num->str_x += move_x;
	if (keycode == 65362)
		(*ap).num->str_y += move_y;
	if (keycode == 65364)
		(*ap).num->str_y -= move_y;
	fractal_check_dot((*ap).mlx, (*ap).num);
	//mlx_put_image_to_window(mlx, win, img, 0, 0);
}
