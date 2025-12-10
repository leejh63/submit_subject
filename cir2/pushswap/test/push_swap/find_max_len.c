/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   find_max_len.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 12:18:45 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 13:25:43 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

void	set_table(long long *max_table, long long num)
{
	long long	i;

	i = 0;
	while (max_table[i] != -1)
	{
		max_table[i] = num;
		i++;
	}
}

long long	find_max_last(long long *max_table, long long size)
{
	long long	i;
	long long	max;

	i = 0;
	max = -1;
	while (i < size)
	{
		if (max < max_table[i])
			max = max_table[i];
		i++;
	}
	return (max);
}

long long	insert_max_table(long long *max_table, long long num)
{
	long long	i;

	i = 0;
	while (max_table[i] != -1)
		i++;
	max_table[i] = num;
	return (i);
}

long long	change_max_table(long long *max_table, long long num)
{
	long long	i;

	i = 0;
	while (max_table[i] < num)
		i++;
	max_table[i] = num;
	return (i);
}

void	find_max_len(long long *c_tab, long long *m_tab, long long *i_tab)
{
	long long	i;
	long long	total_size;

	i = 0;
	total_size = table_size(c_tab);
	set_table(m_tab, -1);
	set_table(i_tab, -1);
	m_tab[i] = c_tab[i];
	while (c_tab[i] != -1)
	{
		if (c_tab[i] > find_max_last(m_tab, total_size))
			i_tab[i] = insert_max_table(m_tab, c_tab[i]);
		else
			i_tab[i] = change_max_table(m_tab, c_tab[i]);
		i++;
	}
}
