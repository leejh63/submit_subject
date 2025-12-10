/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dda_new.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/25 15:44:49 by kkeum             #+#    #+#             */
/*   Updated: 2025/08/25 15:44:50 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

void	init_vars(t_ray *r, double ray_x, double ray_y, t_param *p)
{
	r->pos_x = p->v.xpos;
	r->pos_y = p->v.ypos;
	r->map_x = (int) floor(r->pos_x);
	r->map_y = (int) floor(r->pos_y);
	//printf("r->pos_x %.2f, r->pos_y%.2f, r->map_x %d, r->map_y %d\n", r->pos_x, r->pos_y, r->map_x, r->map_y);
	if (ray_x == 0)
		r->ddx = 1e30;
	else
		r->ddx = fabs(1 / ray_x);
	if (ray_y == 0)
		r->ddy = 1e30;
	else
		r->ddy = fabs(1 / ray_y);
	r->hit = 0;
}

void	set_step_sd(t_ray *r, double ray_x, double ray_y)
{
	if (ray_x < 0)
	{
		r->step_x = -1;
		r->sdx = (r->pos_x - r->map_x) * r->ddx;
	}
	else
	{
		r->step_x = 1;
		r->sdx = (r->map_x + 1.0 - r->pos_x) * r->ddx;
	}
	if (ray_y < 0)
	{
		r->step_y = -1;
		r->sdy = (r->pos_y - r->map_y) * r->ddy;
	}
	else
	{
		r->step_y = 1;
		r->sdy = (r->map_y + 1.0 - r->pos_y) * r->ddy;
	}
}

void	set_texture_img(t_param *p, t_ray *r, double ray_x, double ray_y)
{
	if (r->side == 0)
	{
		r->hit_wall = r->pos_y + r->dist * ray_y;
		if (r->step_x < 0)
		{
			r->texture = p->rsrc.w;
			r->hit_wall *= -1;
		}
		else
			r->texture = p->rsrc.e;
	}
	else
	{
		r->hit_wall = r->pos_x + r->dist * ray_x;
		if (r->step_y < 0)
			r->texture = p->rsrc.n;
		else
		{
			r->texture = p->rsrc.s;
			r->hit_wall *= -1;
		}
	}
}

int	advance_ray(t_ray *r, t_param *p, double ray_x, double ray_y, double *door_dist)
{
	while (!r->hit)
	{
		if (r->sdx < r->sdy)
		{
			r->sdx += r->ddx;
			r->map_x += r->step_x;
			r->side = 0;
		}
		else
		{
			r->sdy += r->ddy;
			r->map_y += r->step_y;
			r->side = 1;
		}
		if (fmin(fabs(r->sdx), fabs(r->sdy)) > 100)
			return (1);
		else if (in_range(r->map_x, p->map.w, r->map_y, p->map.h))
		{
			
			if (p->map.map[r->map_y][r->map_x] == '1')
				r->hit = 1;
			fill_door_info(r, p, ray_x, ray_y, door_dist);
			//
			//obj_fill_info()
			//
		}
	}
	return (0);
}

void	dda_ray_cast(double ray_x, double ray_y, t_param *p, int pix_x)
{
	t_ray	r;
	double	rayxy[2];
	double  door_dist = -1;

	p->dr.door = 0;
	p->dr.dcount = 0;
	rayxy[0] = ray_x;
	rayxy[1] = ray_y;
	init_vars(&r, ray_x, ray_y, p);
	set_step_sd(&r, ray_x, ray_y);
	if (advance_ray(&r, p, ray_x, ray_y, &door_dist))
	{
		draw_empty_column(p, pix_x);
		return ;
	}
	if (r.side == 0)
		r.dist = (r.map_x - r.pos_x + (1 - r.step_x) / 2) / ray_x;
	else
		r.dist = (r.map_y - r.pos_y + (1 - r.step_y) / 2) / ray_y;
	if (p->dda_count < WIN_W)
	{
		double L = (door_dist > 0.0) ? door_dist : r.dist;
		p->dda_dis[p->dda_count] = L;
		p->dda_count++;
	}
	set_texture_img(p, &r, ray_x, ray_y);
	r.hit_wall -= floor(r.hit_wall);
	draw_column(&r, p, (int)floor(r.hit_wall * IMG_SIZE), pix_x);
	draw_door(p, r, pix_x, rayxy);
	//draw_obj(p, r, pix_x, rayxy);
}	
