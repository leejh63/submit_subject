/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   door.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/26 16:57:37 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/08/26 16:57:38 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

int	door_dis(t_ray *r, t_door *dr, double ray_x, double ray_y)
{
	double	t;
	double	hit;

	dr->d_si[dr->dcount] = r->side;
	if (r->side == 0) /* x = map_x + 0.5 */
	{
		if (ray_x == 0.0)
			return (0);
		t = ((double)r->map_x + 0.5 - r->pos_x) / ray_x;
		if (!(t > 0.0))
			return (0);
		hit = r->pos_y + t * ray_y; /* 교차 y */
		/* 교차점이 현재 셀 내부일 때만 유효 */
		if (hit < (double)r->map_y || hit >= (double)r->map_y + 1.0)
			return (0);
	}
	else /* y = map_y + 0.5 */
	{
		if (ray_y == 0.0)
			return (0);
		t = ((double)r->map_y + 0.5 - r->pos_y) / ray_y;
		if (!(t > 0.0))
			return (0);
		hit = r->pos_x + t * ray_x; /* 교차 x */
		if (hit < (double)r->map_x || hit >= (double)r->map_x + 1.0)
			return (0);
	}
	dr->d_di[dr->dcount] = t;
	return (1);
}

void	door_stat(t_ray *r, t_door *dr, char **map)
{
	if (map[r->map_y][r->map_x] == '2')
		dr->d_st[dr->dcount] = 0;
	else
		dr->d_st[dr->dcount] = 1;
	dr->dcount += 1;
}

int	fill_door_info(t_ray *r, t_param *p, double ray_x, double ray_y, double *door_dist)
{
	char	**map;
	t_door	*dr;

	dr = &p->dr;
	map = p->map.map;
	if (map[r->map_y][r->map_x] == '2' || map[r->map_y][r->map_x] == '3')
	{
		if (dr->dcount < DSEE)
		{
			if (*door_dist < 0 && map[r->map_y][r->map_x] == '2')
			{
				if (r->side == 0)
					*door_dist = (r->map_x - r->pos_x + (1 - r->step_x) / 2) / ray_x;
				else
					*door_dist = (r->map_y - r->pos_y + (1 - r->step_y) / 2) / ray_y;
			}
			if (door_dis(r, dr, ray_x, ray_y)) /* 셀 내부 교차일 때만 */
			{
				dr->door = 1;
				door_stat(r, dr, map);
			}
		}
	}
	return (0);
}

int	get_door_addr(t_door *dr, char **door_addr, int door_ind)
{
	if (dr->d_st[door_ind] == 0)
		*door_addr = mlx_get_data_addr(dr->d_c, \
	&dr->d_bpp, &dr->d_line, &dr->d_end);
	else
		*door_addr = mlx_get_data_addr(dr->d_o, \
	&dr->d_bpp, &dr->d_line, &dr->d_end);
	dr->d_bpp = dr->d_bpp / 8;
	return (0);
}

int	draw_door(t_param *p, t_ray r, int pix_x, double *ray_xy)
{
	t_dors	dors;
	t_door	*dr;
	int		ind;

	dr = &p->dr;
	if (dr->door)
	{
		ind = dr->dcount - 1;
		while (ind >= 0)
		{
			//if (dr->d_di[ind] < 5.5)
			{
				get_door_addr(dr, &dors.daddr, ind);
				dors.ptx = pix(dr, r, ray_xy, ind);
				imgetaddr(p, dors.i, &dors.iaddr);
				dors.dty = (double)IMG_SIZE / (double)door_h(p, ind, dors.t);
				color_door(dr, dors, pix_x);
			}
			ind -= 1;
		}
	}
	return (0);
}
