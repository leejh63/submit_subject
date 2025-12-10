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
	{
		p->v.theta -= deg;
		if (p->v.theta < 0)
			p->v.theta += M_PI * 2;
	}
	if (p->key[RIGHT])
	{
		p->v.theta += deg;
		if (p->v.theta >= M_PI * 2)
			p->v.theta -= M_PI * 2;
	}
	return (0);
}

int	move_key(t_param *p, const double d_move)
{
	double	co_si[2];

	co_si[0] = cos(p->v.theta);
	co_si[1] = sin(p->v.theta);
	if (p->key[W])
	{
		p->v.xpos = p->v.xpos + co_si[0] * d_move;
		p->v.ypos = p->v.ypos + co_si[1] * d_move;
	}
	if (p->key[S])
	{
		p->v.xpos = p->v.xpos - co_si[0] * d_move;
		p->v.ypos = p->v.ypos - co_si[1] * d_move;
	}
	if (p->key[A])
	{
		p->v.xpos = p->v.xpos + co_si[1] * d_move;
		p->v.ypos = p->v.ypos - co_si[0] * d_move;
	}
	if (p->key[D])
	{
		p->v.xpos = p->v.xpos - co_si[1] * d_move;
		p->v.ypos = p->v.ypos + co_si[0] * d_move;
	}
	return (0);
}

void	move(t_param *p)
{
	const double	deg = 0.05;
	const double	d_move = 0.035;

	dir_key(p, deg);
	move_key(p, d_move);
	return ;
}
