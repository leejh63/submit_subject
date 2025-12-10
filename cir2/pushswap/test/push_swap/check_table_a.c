/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_table_a.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 11:59:05 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 15:45:53 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

int	sort_two_a(t_stack **head)
{
	sa(head);
	return (0);
}

int	sort_three_a(t_stack **head, long long *table)
{
	if (table[0] < table[1])
	{
		if (table[0] > table[2])
			return (rra(head), 0);
		else if (table[1] > table[2])
			return (sa(head), ra(head), 0);
	}
	else
	{
		if (table[0] < table[2])
			return (sa(head), 0);
		else
		{
			if (table[1] < table[2])
				return (ra(head), 0);
			else
				return (sa(head), rra(head), 0);
		}
	}
	return (0);
}

int	check_table_a(t_stack **a, t_stack **b, long long a_size, long long *table)
{
	if (a_size == 2)
		return (sort_two_a(a));
	else if (a_size == 3)
		return (sort_three_a(a, table));
	else if (a_size <= 50)
		return (sort_up_three_a(a, b, table));
	return (1);
}
