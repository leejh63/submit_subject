/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   find_maxlen_init.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 12:16:17 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 13:27:22 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

long long	table_size(long long *table)
{
	long long	i;

	i = 0;
	while (table[i] != -1)
		i++;
	return (i);
}

int	find_maxlen_init(long long *c_table, long long *i_table, long long t_size)
{
	long long	*max_table;
	long long	*index_table;
	long long	max_len;
	long long	i;

	if (set_init_table(t_size, &max_table))
		return (tmp_error(100));
	if (set_init_table(t_size, &index_table))
		return (free(max_table), tmp_error(100));
	// 최장 부분 길이 수열 구하는 함수
	find_max_len(c_table, max_table, index_table);
	max_len = table_size(max_table) - 1;
	i = table_size(index_table);
	while (--i)
	{
		if (index_table[i] == max_len)
		{
			i_table[i] = c_table[i];
			max_len--;
		}
	}
	return (free(max_table), free(index_table), 0);
}
