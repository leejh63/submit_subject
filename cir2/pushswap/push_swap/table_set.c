/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   table_set.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 11:55:34 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 15:37:21 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

long long	stack_size(t_stack *head)
{
	long long	i;
	t_stack		*stack;

	i = 0;
	stack = head;
	while (stack)
	{
		stack = stack -> next;
		i++;
	}
	return (i);
}

int	make_init_table(long long size_stack, long long **tab)
{
	int			i;

	i = -1;
	*tab = malloc(sizeof(long long) * (size_stack + 1));
	if (!*tab)
		return (tmp_error(8));
	while (++i <= size_stack)
		(*tab)[i] = -1;
	return (0);
}

int	fill_table(t_stack *head, long long *table)
{
	t_stack	*stack;
	int		i;

	i = 0;
	stack = head;
	while (stack)
	{
		table[i] = stack -> num;
		stack = stack -> next;
		i++;
	}
	table[i] = -1;
	return (0);
}

int	check_atable(long long *a_table)
{
	int	i;

	i = 1;
	while (a_table[i] != -1)
	{
		if (a_table[i - 1] > a_table[i])
			return (0);
		i++;
	}
	return (1);
}

int	table_set(t_stack **ahead, long long **a_table, long long **b_table)
{
	long long	size_stack;

	size_stack = stack_size(*ahead);
	if (make_init_table(size_stack, a_table))
		return (tmp_error(9));
	if (make_init_table(size_stack, b_table))
	{
		free(*a_table);
		*a_table = NULL;
		return (tmp_error(10));
	}
	fill_table(*ahead, *a_table);
	if (check_atable(*a_table))
		return (1);
	return (0);
}
