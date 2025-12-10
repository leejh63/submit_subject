/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sorting_func.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 12:22:24 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 15:05:50 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

int	div_func(int div, int size_stack)
{
	int	div_num;

	div_num = (25 * size_stack + 1500) / 1000;
	return (div - size_stack / div_num);
}

long long	init_size(long long *init_table, long long init_size)
{
	long long	i;
	long long	count;

	i = 0;
	count = 0;
	while (i < init_size)
	{
		if (init_table[i] != -1)
			count++;
		i++;
	}
	return (count);
}

int	find_atop(long long *init_table, long long atop, long long size_table)
{
	long long	i;

	i = 0;
	while (i < size_table)
	{
		if (init_table[i] == atop)
			return (0);
		i++;
	}
	return (1);
}

void	sorting_func(t_stack **a, t_stack **b, long long *it, long long *b_tab)
{
	long long	k;
	long long	div;
	long long	t_size;

	t_size = stack_size(*a) + stack_size(*b);
	div = t_size;
	while (stack_size(*a) != init_size(it, t_size))
	{
		k = -1;
		div = div_func(div, t_size);
		while (++k < stack_size(*a) && stack_size(*a) != init_size(it, t_size))
		{
			update_table(*b, b_tab);
			if (find_atop(it, (*a)->num, t_size))
			{
				if (div <= (*a)->num)
					move_b_init(a, b, b_tab);
				else
					ra(a);
			}
			else
				ra(a);
		}
	}
}
