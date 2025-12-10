/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   moves.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/06 16:41:52 by kkeum             #+#    #+#             */
/*   Updated: 2025/08/25 15:44:58 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

int	dir_key(t_param *p, const double deg)
{
	if (p->key[UP])
	{
		if (p->v.verti < M_PI_4 + M_PI / 6)
			p->v.verti += deg;
	}
	if (p->key[DOWN])
	{
		if (p->v.verti > M_PI / 6)
			p->v.verti -= deg;
	}
	if (p->key[LEFT])
		p->v.theta -= deg;
	if (p->key[RIGHT])
		p->v.theta += deg;
	return (0);
}

static void	collis(t_param *p, double x_d, double y_d)
{
	int		mapx;
	int		mapy;
	char	**map;

	map = p->map.map;
	mapx = (int) floor(p->v.xpos + x_d);
	mapy = (int) floor(p->v.ypos + y_d);
	if (map[mapy][mapx] == '0' || map[mapy][mapx] == '3')
	{
		p->v.xpos = p->v.xpos + x_d;
		p->v.ypos = p->v.ypos + y_d;
	}
}

int	change_door(t_param *p, const double d_move)
{
	int		posxy[4];
	char	**map;

	map = p->map.map;
	posxy[0] = (int) floor(p->v.xpos);
	posxy[1] = (int) floor(p->v.ypos);
	if (map[posxy[1]][posxy[0]] == '0' || map[posxy[1]][posxy[0]] == '3')
	{
		posxy[2] = (int)(p->v.xpos + cos(p->v.theta) * d_move);
		posxy[3] = (int)(p->v.ypos + sin(p->v.theta) * d_move);
		if ((posxy[0] != posxy[2]) || (posxy[1] != posxy[3]))
		{
			if (map[posxy[3]][posxy[2]] == '2')
				map[posxy[3]][posxy[2]] = '3';
			else if (map[posxy[3]][posxy[2]] == '3')
				map[posxy[3]][posxy[2]] = '2';
		}
	}
	p->key[SPACE] = 0;
	return (0);
}

int	move_key(t_param *p, const double d_move)
{
	double	co_si[2];

	co_si[0] = cos(p->v.theta) * d_move;
	co_si[1] = sin(p->v.theta) * d_move;
	if (p->key[SPACE])
		change_door(p, d_move + 0.7);
	if (p->key[W])
		collis(p, co_si[0], co_si[1]);
	if (p->key[S])
		collis(p, -co_si[0], -co_si[1]);
	if (p->key[A])
		collis(p, co_si[1], -co_si[0]);
	if (p->key[D])
		collis(p, -co_si[1], +co_si[0]);
	return (0);
}

void	move(t_param *p)
{
	const double	deg = 0.05;
	const double	d_move = 0.035;

	dir_key(p, deg);
	move_key(p, d_move);
	p->v.theta = fmod(p->v.theta, M_PI * 2);
	return ;
}
