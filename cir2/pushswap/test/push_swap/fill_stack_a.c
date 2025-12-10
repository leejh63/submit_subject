/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fill_stack_a.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 11:49:31 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 13:23:59 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

long long	find_ind(long long *sort_table, long long box_num)
{
	long long	i;

	i = 0;
	while (box_num != sort_table[i])
		i++;
	return (i);
}

int	fill_stack_a(t_stack **ahead, long long *sort_table, char **numbox)
{
	int			i;
	t_stack		*new;

	i = -1;
	while (numbox[++i])
	{
		new = new_num(find_ind(sort_table, l_ft_atoi(numbox[i])));
		if (!new)
			return (free(sort_table), 1);
		add_b(ahead, new);
	}
	return (0);
}
