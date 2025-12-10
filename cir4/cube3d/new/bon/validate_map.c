/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   validate_map.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/06 16:42:00 by kkeum             #+#    #+#             */
/*   Updated: 2025/08/25 15:45:02 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

int	check_characters(t_param *p)
{
	int		x;
	int		y;

	y = 0;
	while (y < p->map.h)
	{
		x = 0;
		while (x < p->map.w)
		{
			if (p->map.map[y][x] != ' ' && p->map.map[y][x] != '0' && \
				p->map.map[y][x] != '1' && p->map.map[y][x] != '2')
				return (0);
			x++;
		}
		y++;
	}
	return (1);
}

void	update_param(t_param *p, int x, int y, char c)
{
	p->v.xpos = x + 0.5;
	p->v.ypos = y + 0.5;
	p->v.verti = M_PI_4;
	p->v.pre_mouse_x = WIN_W / 2;
	p->v.pre_mouse_y = WIN_H / 2;
	if (c == 'N')
		p->v.theta = M_PI_2 * 3;
	else if (c == 'W')
		p->v.theta = M_PI;
	else if (c == 'S')
		p->v.theta = M_PI_2;
	else if (c == 'E')
		p->v.theta = 0;
}

int	check_start(t_param *p)
{
	int		x;
	int		y;
	int		start_pos;

	start_pos = 0;
	y = -1;
	while (++y < p->map.h)
	{
		x = -1;
		while (++x < p->map.w)
		{
			if (p->map.map[y][x] == 'N' || p->map.map[y][x] == 'E' || \
				p->map.map[y][x] == 'W' || p->map.map[y][x] == 'S')
			{
				update_param(p, x, y, p->map.map[y][x]);
				p->map.map[y][x] = '0';
				start_pos++;
			}
		}
	}
	return (start_pos);
}

void	validate_map(t_param *p)
{
	int	start_pos;

	start_pos = check_start(p);
	if (start_pos == 0)
		ft_exit(p, "Error\nZero or multiple starting positions in the map.", 1);
	else if (start_pos > 1)
		ft_exit(p, "Error\nMultiple starting positions in the map.", 1);
	if (!check_characters(p))
		ft_exit(p, "Error\nInvalid Charaters in the map.", 1);
	if ((!is_surrounded_by_wall(&(*p).map)) || (!zero_surroundings(&(*p).map)))
		ft_exit(p, "Error\nMap not surrounded by wall.", 1);
}
