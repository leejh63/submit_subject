/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort_logic.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 11:51:35 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 14:36:45 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

int	set_init_table(long long size_stack, long long **tab)
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

void	move_to_a(t_stack **ahead, t_stack **bhead, long long *a_table)
{
	long long	size_stack;

	size_stack = stack_size(*bhead);
	update_table(*ahead, a_table);
	while (size_stack > 0)
	{
		find_set_stack(ahead, bhead, a_table, 'a');
		size_stack--;
	}
}

void	move_to_sort_a(t_stack **ahead, t_stack **bhead, long long *table)
{
	long long	i;

	update_table(*ahead, table);
	i = find_table_ind(table, 0);
	move_to_short_a(ahead, bhead, i);
}

int	sort_logic(t_stack **ahead, t_stack **bhead, long long a_size)
{
	int			i;
	long long	*a_table;
	long long	*b_table;
	long long	*init_table;

	i = table_set(ahead, &a_table, &b_table);
	if (i)
		return (free(a_table), free(b_table), 1);
	if (!check_table_a(ahead, bhead, a_size, a_table))
		return (free(a_table), free(b_table), 1);
	if (set_init_table(a_size, &init_table))
		return (free(a_table), free(b_table), 1);
	if (find_maxlen_init(a_table, init_table, a_size))
		return (free(a_table), free(b_table), free(init_table), 1);
	sorting_func(ahead, bhead, init_table, b_table);
	move_to_a(ahead, bhead, a_table);
	move_to_sort_a(ahead, bhead, a_table);
	return (free(a_table), free(b_table), free(init_table), 0);
}
