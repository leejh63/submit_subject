/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   draw.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/26 12:12:58 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/08/26 12:13:00 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

void	draw_column(t_ray *r, t_param *p, int tex_x, int pix_x)
{
	double	hit_y;
	t_color	color;
	int		i;
	int		tex_y;

	static double	d_jump = 0.00005;
	static int	jsig;
	static double jump;
	if(p->key[SPACE] == 1)
	{
		if (!jsig)
		{
			jump += d_jump;
			if (jump > 0.5)
				jsig = 1;
		}
		else
		{
			jump -= d_jump;
			if (jump < 0.001)
			{
				p->key[SPACE] = 0;
				jump = 0;
				jsig = 0;
			}
		}
	}
		

	i = 0;
	while (i < WIN_H)
	{
		hit_y = (0.5 - jump) + r->dist * (i - WIN_H / 2.0) / (WIN_W / 2);
		tex_y = (int)floor(hit_y * IMG_SIZE);
		if (hit_y < 0.0)
			color = p->map.c;
		else if (hit_y > 1)
			color = p->map.f;
		else
			color = export_color(r->texture, tex_x, tex_y);
		ft_pixel_put(p, pix_x, i, color.i);
		i++;
	}
}

void	draw_empty_column(t_param *p, int pix_x)
{
	int	y;

	y = 0;
	while (y < WIN_H / 2)
	{
		ft_pixel_put(p, pix_x, y, p->map.c.i);
		y += 1;
	}
	while (y < WIN_H)
	{
		ft_pixel_put(p, pix_x, y, p->map.f.i);
		y += 1;
	}
	return ;
}
