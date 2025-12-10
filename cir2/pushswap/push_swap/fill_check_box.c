/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fill_check_box.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 11:52:35 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 13:22:45 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

int	is_not_num(char **numbox)
{
	int	i;
	int	j;

	i = -1;
	while (numbox[++i])
	{
		j = 0;
		while (numbox[i][j])
		{
			if (numbox[i][j] == '-')
				j++;
			if (!ft_isdigit(numbox[i][j]))
				return (1);
			while (ft_isdigit(numbox[i][j]))
				j++;
			if (numbox[i][j])
				return (1);
		}
	}
	return (0);
}

int	fill_check_box(int argc, char ***numbox, char **argv)
{
	if (argc == 1)
		return (tmp_error(1));
	(*numbox) = &(argv[1]);
	if (argc == 2)
	{
		(*numbox) = ft_split(argv[1], ' ');
		if (!*numbox)
			return (tmp_error(2));
	}
	if (is_not_num(*numbox))
		return (tmp_error(3));
	return (0);
}
