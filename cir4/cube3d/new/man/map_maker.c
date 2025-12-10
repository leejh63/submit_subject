/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   map_maker.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/25 15:44:56 by kkeum             #+#    #+#             */
/*   Updated: 2025/08/25 15:44:56 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

char	*fill_space(char *s, size_t size)
{
	int		len;
	char	*new_str;

	len = ft_strlen(s);
	if (ft_strchr(s, '\n'))
		len--;
	new_str = ft_calloc(size + 1, sizeof(char));
	if (new_str == NULL)
		return (NULL);
	ft_memcpy(new_str, s, len);
	ft_memset(new_str + len, ' ', size - len);
	free(s);
	return (new_str);
}

void	adjust_size(t_param *p)
{
	int		i;
	size_t	max_w;
	size_t	cur_w;

	max_w = 0;
	i = -1;
	while (++i < p->map.h)
	{
		cur_w = ft_strlen(p->map.map[i]);
		if (ft_strchr(p->map.map[i], '\n'))
			cur_w--;
		if (cur_w > max_w)
			max_w = cur_w;
	}
	i = -1;
	while (++i < p->map.h)
	{
		p->map.map[i] = fill_space(p->map.map[i], max_w);
		if (p->map.map[i] == NULL)
			ft_exit(p, "malloc fail", 1);
	}
	p->map.w = max_w;
}
