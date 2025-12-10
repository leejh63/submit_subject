/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   doors_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/26 18:30:31 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/08/26 18:30:32 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

int	pix(t_door *dr, t_ray r, const double *ray_xy, int door_ind)
{
	double	door_wall;
	if (dr->d_si[door_ind] == 0)
	{
		door_wall = r.pos_y + dr->d_di[door_ind] * ray_xy[1];
		if (r.step_x < 0)
			door_wall *= -1;
	}
	else
	{
		door_wall = r.pos_x + dr->d_di[door_ind] * ray_xy[0];
		if (r.step_y > 0)
			door_wall *= -1;
	}
	door_wall -= floor(door_wall);
	//printf("door_wall: %.2f\n", door_wall);
	return ((int)(door_wall * IMG_SIZE));
}

int	imgetaddr(t_param *p, int *img_int, char **img_addr)
{
	*img_addr = mlx_get_data_addr(p->img, &img_int[0], \
		&img_int[1], &img_int[2]);
	img_int[0] = img_int[0] / 8;
	return (0);
}

int	door_h(t_param *p, int door_ind, int *t)
{
	t_door	*dr;
	int		tmp_window_h;
	int		door_h;

	/*/
	int		jump_offset;
	static double	d_jump = 0.00005;
	static int	jsig;
	static double jump;
	if(p->key[CTRL] == 1)
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
			if (jump < 0.1)
			{
				p->key[CTRL] = 0;
				jump = 0;
				jsig = 0;
			}
		}
	}
	jump_offset = (int)(jump * (WIN_H / dr->d_di[door_ind] / 2));
	/*/
	dr = &p->dr;

	tmp_window_h = (WIN_H + (int)(WIN_H * (sin(p->v.verti) - cos(p->v.verti))));
	door_h = WIN_H / dr->d_di[door_ind] / 2;
	t[0] = tmp_window_h / 2 - door_h / 2;// + jump_offset;
	t[1] = tmp_window_h / 2 + door_h / 2;// + jump_offset;
	return (door_h);
}

int	fill_door(t_door *dr, t_dors d, int texy, int pix_x)
{
	unsigned int	dcol;
	unsigned int	*diaddr;

	diaddr = (unsigned int *)(d.iaddr + d.t[0] * d.i[1] + pix_x * d.i[0]);
	dcol = *(unsigned int *)(d.daddr + texy * dr->d_line + d.ptx * dr->d_bpp);
	if ((dcol & 0xFFFFFF) != 0x000000)
	{
		if (pix_x >= 0 && pix_x < WIN_W && d.t[0] >= 0 && d.t[0] < WIN_H)
			*diaddr = dcol;
	}
	return (0);
}

int	color_door(t_door *dr, t_dors dors, int pix_x)
{
	int		texy;
	double	tex_pos;

	texy = 0;
	tex_pos = 0;
	if (dors.t[0] < 0)
	{
		tex_pos += -dors.t[0] * dors.dty;
		dors.t[0] = 0;
	}
	if (dors.t[1] >= WIN_H)
		dors.t[1] = WIN_H - 1;
	while (dors.t[0] <= dors.t[1])
	{
		texy = (int)tex_pos;
		tex_pos += dors.dty;
		fill_door(dr, dors, texy, pix_x);
		dors.t[0] += 1;
	}
	return (0);
}
