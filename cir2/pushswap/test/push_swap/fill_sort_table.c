/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fill_sort_table.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 11:23:05 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 13:23:14 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

const char	*ft_skipspace(const char *nptr)
{
	while (*nptr == ' ' || (*nptr >= '\t' && *nptr <= '\r'))
		nptr++;
	return (nptr);
}

long long	l_ft_atoi(const char *nptr)
{
	int				minus;
	long long		num;

	if (!nptr)
		return (0);
	minus = 1;
	nptr = ft_skipspace(nptr);
	if (*nptr == '-' || *nptr == '+')
	{
		if (*nptr == '-')
			minus = -1;
		nptr++;
	}
	num = 0;
	while (ft_isdigit(*nptr))
	{
		num = num * 10 + (*nptr - '0');
		nptr++;
	}
	return (minus * num);
}

void	table_sort(long long *table, int table_size)
{
	int			i;
	int			j;
	long long	tmp;

	i = 0;
	while (i < table_size)
	{
		j = i + 1;
		while (j < table_size)
		{
			if (table[i] > table[j])
			{
				tmp = table[j];
				table[j] = table[i];
				table[i] = tmp;
			}
			j++;
		}
		i++;
	}
}

int	table_check(long long *sort_table, int table_size)
{
	int	i;

	i = 0;
	while (++i < table_size)
		if (sort_table[i - 1] == sort_table[i])
			return (1);
	return (0);
}

int	fill_sort_table(char **numbox, long long *init_table, int table_size)
{
	int			i;
	long long	box_num;

	i = -1;
	while (++i < table_size)
	{
		box_num = l_ft_atoi(numbox[i]);
		if (box_num < -2147483648 || box_num > 2147483647)
			return (free(init_table), tmp_error(5));
		init_table[i] = box_num;
	}
	table_sort(init_table, table_size);
	if (table_check(init_table, table_size))
		return (free(init_table), tmp_error(6));
	return (0);
}
