/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fire.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 20:07:30 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/08/27 20:07:32 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

int	draw_fire(t_param *p)
{
	struct timeval	tv;
	const void *frame[4] = {p->rsrc.one, p->rsrc.two, p->rsrc.thr, p->rsrc.fur};
	gettimeofday(&tv, NULL);
	int start_x = WIN_W - SPRITE_SIZE;
	int start_y = WIN_H - SPRITE_SIZE;
	int ind = (tv.tv_usec / 100000) % 4;
	int i_bpp, i_line, iend;
	int f_bpp, f_line, fend;
	char *img_addr = mlx_get_data_addr(p->img, &i_bpp, &i_line, &iend);
	char *fire_tex = mlx_get_data_addr((void *)frame[ind], &f_bpp, &f_line, &fend);
	int f_y = 0;
	while (f_y < SPRITE_SIZE)
	{
		int f_x = 0;
		while (f_x < SPRITE_SIZE)
		{
			unsigned int color = *(unsigned int*)(fire_tex + f_x * (f_bpp / 8) + f_y * f_line);
			unsigned int *img = (unsigned int*)(img_addr + (start_x + f_x) * (i_bpp / 8) + (start_y + f_y) * i_line);
			if ((color & 0xFFFFFF) != 0x000000)
				*img = color;
			f_x++;
		}
		f_y++;
	}
	return (0);
}



















