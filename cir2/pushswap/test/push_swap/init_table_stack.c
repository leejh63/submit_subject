/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_table_stack.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 11:53:21 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 13:29:15 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

int	box_size(char **numbox)
{
	int	i;

	i = 0;
	while (numbox[i])
		i++;
	return (i);
}

int	init_table_stack(char **numbox, t_stack **ahead)
{
	int			table_size;
	long long	*init_table;

	table_size = box_size(numbox);
	init_table = malloc(sizeof(long long) * table_size);
	if (!init_table)
		return (tmp_error(4));
	if (fill_sort_table(numbox, init_table, table_size))
		return (56);
	if (fill_stack_a(ahead, init_table, numbox))
		return (tmp_error(7));
	free(init_table);
	return (0);
}
