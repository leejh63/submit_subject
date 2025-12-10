/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_utils.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/26 14:53:53 by kkeum             #+#    #+#             */
/*   Updated: 2025/07/26 14:53:54 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

int	validate_args(int argc, char **argv)
{
	char	*extension;

	while (1)
	{
		if (argc != 2)
			break ;
		extension = ft_strrchr(argv[1], '.');
		if (extension && ft_strncmp(ft_strrchr(argv[1], '.'), ".cub", 5))
			break ;
		return (0);
	}
	ft_putstr_fd("Usage: ", 2);
	ft_putstr_fd(argv[0], 2);
	ft_putendl_fd(" [map.cub(filename)]", 2);
	return (1);
}

int	is_all_white(char *s)
{
	while (*s == ' ' || *s == '\n')
		s++;
	return (!*s);
}

int	in_range(int x, int max_x, int y, int max_y)
{
	if (x >= 0 && x < max_x && y >= 0 && y < max_y)
		return (1);
	return (0);
}
