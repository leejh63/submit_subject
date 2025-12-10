/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort_up_three_a.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 12:01:02 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 15:03:07 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

void	update_table(t_stack *head, long long *table)
{
	long long	i;

	i = 0;
	while (head)
	{
		table[i] = head -> num;
		head = head -> next;
		i++;
	}
	table[i] = -1;
}

long long	find_table_ind(long long *table, long long num)
{
	long long	i;

	i = 0;
	while (table[i] != -1)
	{
		if (num == table[i])
			return (i);
		i++;
	}
	return (0);
}

int	sort_up_three_a(t_stack **ahead, t_stack **bhead, long long *table)
{
	long long	size_stack;

	size_stack = stack_size(*ahead);
	while (size_stack != 3)
	{
		pb(ahead, bhead);
		size_stack = stack_size(*ahead);
	}
	update_table(*ahead, table);
	sort_three_a(ahead, table);
	size_stack = stack_size(*bhead);
	while (size_stack > 0)
	{
		find_set_stack(ahead, bhead, table, 'a');
		size_stack--;
	}
	move_to_sort_a(ahead, bhead, table);
	return (0);
}
