/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   move_b_init.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 12:25:22 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 13:29:46 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

int	sort_three_b(t_stack **ahead, t_stack **bhead, long long *b_table)
{
	long long	a_top;

	a_top = (*ahead)->num;
	update_table(*bhead, b_table);
	pb(ahead, bhead);
	if (b_table[0] > b_table[1])
	{
		if (a_top < b_table[0] && a_top > b_table[1])
			sb(bhead);
	}
	else
	{
		if (a_top < b_table[0] || a_top > b_table[1])
			sb(bhead);
	}
	return (2);
}

int	sort_up_three_b(t_stack **ahead, t_stack **bhead, long long *b_table)
{
	update_table(*bhead, b_table);
	find_set_stack(ahead, bhead, b_table, 'b');
	return (0);
}

int	move_b_init(t_stack **ahead, t_stack **bhead, long long *b_table)
{
	long long	b_size;

	b_size = stack_size(*bhead);
	if (b_size <= 1)
		return (pb(ahead, bhead), 1);
	if (b_size == 2)
		return (sort_three_b(ahead, bhead, b_table), 3);
	if (b_size >= 3)
		return (sort_up_three_b(ahead, bhead, b_table), 4);
	return (0);
}
