/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   animation.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/26 16:04:04 by kkeum             #+#    #+#             */
/*   Updated: 2025/08/26 16:04:08 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

void	ft_put_trans_img(t_img *scr, t_img *img, int x, int y)
{
	int	*dst;
	int	*src;
	int	i;
	int	j;

	dst = (int *) scr->data;
	src = (int *) img->data;
	j = 0;
	while (j < SPRITE_SIZE)
	{
		i = 0;
		while (i < SPRITE_SIZE)
		{
			if (src[(i) + (j) * SPRITE_SIZE] != -16777216)
				ft_memcpy(&dst[(x + i) + (y + j) * WIN_W], \
				&src[(i) + (j) * SPRITE_SIZE], sizeof(int));
			i++;
		}
		j++;
	}
}

void	draw_animation(t_param *p)
{
	const void		*frame[4] = \
	{p->rsrc.r1, p->rsrc.r2, p->rsrc.r3, p->rsrc.r2};
	const int		h = WIN_H - SPRITE_SIZE;
	struct timeval	tv;
	t_img			*img;

	gettimeofday(&tv, NULL);
	if (p->key[W] ^ p->key[S] || p->key[A] ^ p->key[D])
		img = (t_img *)frame[tv.tv_usec / 250000];
	else
		img = (t_img *)p->rsrc.idle;
	ft_put_trans_img(p->img, img, 0, h);
}
