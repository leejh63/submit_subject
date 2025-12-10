/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   find_set_stack.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 12:02:47 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 13:28:56 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

long long	find_inserta_ind(long long *table, long long num)
{
	long long	i;

	i = 1;
	while (table[i] != -1)
	{
		if (num > table[i - 1] && num < table[i])
			return (i);
		if (table[i - 1] > table[i])
			if (num > table[i - 1] || num < table[i])
				return (i);
		i++;
	}
	return (0);
}

void	move_to_short_a(t_stack **ahead, t_stack **bhead, long long ind)
{
	long long	size_stack;
	long long	count;

	size_stack = stack_size(*ahead);
	if (ind < (size_stack - ind))
	{
		count = ind;
		while (count--)
			rotate(ahead, 'a');
	}
	else
	{
		count = size_stack - ind;
		while (count--)
			rrotate(ahead, 'a');
	}
	if (stack_size(*bhead))
		pa(ahead, bhead);
}

long long	find_insertb_ind(long long *table, long long num)
{
	long long	i;

	i = 1;
	while (table[i] != -1)
	{
		if (table[i - 1] > table[i])
		{
			if (num < table[i - 1] && num > table[i])
				return (i);
		}
		else
		{
			if (num > table[i] || num < table[i - 1])
				return (i);
		}
		i++;
	}
	return (0);
}

void	move_to_short_b(t_stack **a, t_stack **b, long long ind, int pb_i)
{
	long long	size_stack;
	long long	count;

	size_stack = stack_size(*b);
	if (ind < (size_stack - ind))
	{
		count = ind;
		while (count--)
			rotate(b, 'b');
	}
	else
	{
		count = size_stack - ind;
		while (count--)
			rrotate(b, 'b');
	}
	if (pb_i)
		pb(a, b);
}

void	find_set_stack(t_stack **a, t_stack **b, long long *table, char ab)
{
	long long	i;
	long long	num;

	if (ab == 'a')
	{
		num = (*b)->num;
		update_table(*a, table);
		i = find_inserta_ind(table, num);
		move_to_short_a(a, b, i);
		update_table(*a, table);
	}
	else
	{
		num = (*a)->num;
		update_table(*b, table);
		i = find_insertb_ind(table, num);
		move_to_short_b(a, b, i, 1);
		update_table(*b, table);
	}
}
