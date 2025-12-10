/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mlx_struct.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/08 21:27:12 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/10 22:03:36 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	set_args(t_mlx **mlx_st, t_num **num_st, t_ap *ap, char **argv)
{
	if (set_num(num_st, argv))
	{
		mlx_destroy_display((*mlx_st)->mlx);
		return (free((*mlx_st)->mlx), free(mlx_st), 1);
	}
	if (init_mlx(*mlx_st, (*num_st)->width, (*num_st)->height))
		return (free(num_st), 1);
	(*ap).mlx = *mlx_st;
	(*ap).num = *num_st;
	return (0);
}

int	make_mlx(t_mlx **mlx_st)
{
	*mlx_st = malloc(sizeof(t_mlx));
	if (!(*mlx_st))
		return (err_num(5));
	(*mlx_st)->mlx = mlx_init();
	if (!(*mlx_st)->mlx)
		return (free(*mlx_st), err_num(0));
	return (0);
}

int	init_mlx(t_mlx *mlx_st, int width, int height)
{
	int	*bpp;
	int	*line_len;
	int	*endian;

	bpp = &(mlx_st -> bpp);
	line_len = &(mlx_st -> line_len);
	endian = &(mlx_st -> endian);
	mlx_st -> win = mlx_new_window(mlx_st->mlx, width, height, "Fractal");
	if (!mlx_st -> win)
	{
		mlx_destroy_display(mlx_st->mlx);
		return (free(mlx_st->mlx), free(mlx_st), err_num(0));
	}
	mlx_st -> img = mlx_new_image(mlx_st -> mlx, width, height);
	if (!mlx_st -> img)
	{
		mlx_destroy_window(mlx_st->mlx, mlx_st->win);
		mlx_destroy_display(mlx_st->mlx);
		return (free(mlx_st -> mlx), free(mlx_st), err_num(0));
	}
	mlx_st -> data = mlx_get_data_addr(mlx_st -> img, bpp, line_len, endian);
	return (0);
}

int	destroy_mlx(t_mlx *mlx_st)
{
	mlx_destroy_window(mlx_st->mlx, mlx_st->win);
	mlx_destroy_image(mlx_st->mlx, mlx_st->img);
	mlx_destroy_display(mlx_st->mlx);
	return (free(mlx_st->mlx), free(mlx_st), 0);
}
