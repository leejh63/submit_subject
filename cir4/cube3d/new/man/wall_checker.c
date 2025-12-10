/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   wall_checker.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/25 15:45:03 by kkeum             #+#    #+#             */
/*   Updated: 2025/08/25 15:45:04 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

int	recur(t_map *map, int *visited, int x, int y)
{
	static const int	dx[4] = {1, -1, 0, 0};
	static const int	dy[4] = {0, 0, 1, -1};
	int					i;

	visited[x + y * map->w] = 1;
	i = 0;
	while (i < 4)
	{
		if (y + dy[i] >= 0 && y + dy[i] < map->h && \
		x + dx[i] >= 0 && x + dx[i] < map->w)
			if (map->map[y + dy[i]][x + dx[i]] != '1' && \
				!visited[x + dx[i] + (y + dy[i]) * map->w])
				recur(map, visited, x + dx[i], (y + dy[i]));
		i++;
	}
	return (0);
}

int	is_surrounded_by_wall(t_map *map)
{
	int	*visited;
	int	x;
	int	y;

	visited = ft_calloc(sizeof(int), map->w * map->h);
	if (visited == NULL)
		return (perror("malloc failed"), 0);
	y = -1;
	while (++y < map->h)
	{
		x = -1;
		while (++x < map->w)
			if (map->map[y][x] == ' ' && !visited[x + y * map->w])
				recur(map, visited, x, y);
	}
	y = -1;
	while (++y < map->h)
	{
		x = -1;
		while (++x < map->w)
			if (map->map[y][x] != ' ' && visited[x + y * map->w])
				return (free(visited), 0);
	}
	return (free(visited), 1);
}

int	zero_surroundings(t_map *map)
{
	int	i;

	i = 0;
	while (i < map->w)
	{
		if (map->map[0][i] == '0' || map->map[0][i] == '2' \
		|| map->map[map->h - 1][i] == '0')
			return (0);
		i++;
	}
	i = 0;
	while (i < map->h)
	{
		if (map->map[i][0] == '0' || map->map[i][map->w - 1] == '0')
			return (0);
		i++;
	}
	return (1);
}
