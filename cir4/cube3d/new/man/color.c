/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   color.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/25 15:44:39 by kkeum             #+#    #+#             */
/*   Updated: 2025/08/25 15:44:41 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

int	check_color_format(char *str)
{
	int	i;

	while (*str == ' ')
		str++;
	if (*str == '\0')
		return (0);
	i = 0;
	while (i++ < 2)
	{
		if (!ft_isdigit(*str))
			return (0);
		while (*str && ft_isdigit(*str))
			str++;
		if (*str++ != ',')
			return (0);
	}
	if (!ft_isdigit(*str))
		return (0);
	while (*str && ft_isdigit(*str))
		str++;
	while (*str == ' ')
		str++;
	if (*str == '\n' || *str == '\0')
		return (1);
	return (0);
}

t_color	parse_color(char *str, t_param *p)
{
	t_color	c;

	if (!check_color_format(str))
		ft_exit(p, "Error\nWrong color configuration.", 1);
	ft_memset(&c, 0, sizeof(t_color));
	while (*str == ' ')
		str++;
	c.c[R] = ft_atoi(str);
	while (ft_isdigit(*str))
		str++;
	c.c[G] = ft_atoi(++str);
	while (ft_isdigit(*str))
		str++;
	c.c[B] = ft_atoi(++str);
	return (c);
}

void	ft_pixel_put(t_param *p, int x, int y, int color)
{
	t_img	*m;

	if (x >= 0 && x < WIN_W && y >= 0 && y < WIN_H)
	{
		m = p->img;
		ft_memcpy(&m->data[(x + y * WIN_W) * 4], &color, sizeof(int));
	}
}

inline t_color	build_color(int red, int green, int blue)
{
	return ((t_color){{red, green, blue, 0}});
}

t_color	export_color(void	*img, int x, int y)
{
	t_img	*m;
	t_color	c;

	c.i = 0;
	m = img;
	if (x >= 0 && x < IMG_SIZE && y >= 0 && y < IMG_SIZE)
	{
		ft_memcpy(&c, &m->data[(x + y * IMG_SIZE) * 4], sizeof(int));
	}
	return (c);
}
