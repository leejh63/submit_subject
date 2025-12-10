/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dot_pixel.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/10 11:00:06 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/11 14:35:24 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	offset(int x, int y, int line_len, int bpp)
{
	return (y * line_len + x * (bpp / 8));
}

int	color_intensity(int repeat, int max_repeat)
{
	int		color;
	double	ratio;

	color = 0;
	if (repeat == max_repeat)
		return (color);
	ratio = ((double)repeat / max_repeat);
	ratio = pow(ratio, 0.5);
	color = 255 * ratio;
	return (color);
}

void	fractal_check_dot(t_mlx *mlx_st, t_num *num_st)
{
	if (num_st->is_mand == 1)
		dot_pixel(mlx_st, num_st, mandel_fractal);
	else
		dot_pixel(mlx_st, num_st, julia_fractal);
}

int	dot_pixel(t_mlx *mt, t_num *nt, int (*func)(int, int, t_num *))
{
	unsigned char	*img_data;
	int				off;
	int				px;
	int				py;

	py = 0;
	img_data = (unsigned char *)mt->data;
	while (py < nt->height)
	{
		px = 0;
		while (px < nt->width)
		{
			func(px, py, nt);
			off = offset(px, py, mt->line_len, mt->bpp);
			img_data[off + 0] = nt->color * fabs(sin(nt->max_x) + 0.7);
			img_data[off + 1] = nt->color * fabs(sin(nt->max_y) + 0.5);
			img_data[off + 2] = nt->color * fabs(cos(nt->min_y) + 0.3);
			px++;
		}
		py++;
	}
	return (0);
}
