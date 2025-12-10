/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 12:57:07 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 15:46:43 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

int	main(int argc, char **argv)
{
	char		**numbox;
	t_stack		*ahead;
	t_stack		*bhead;

	if (argc == 1)
		return (1);
	ahead = NULL;
	bhead = NULL;
	if (fill_check_box(argc, &numbox, argv))
		return (free_all_stack(argc, numbox, &ahead, &bhead));
	if (init_table_stack(numbox, &ahead))
		return (free_all_stack(argc, numbox, &ahead, &bhead));
	sort_logic(&ahead, &bhead, stack_size(ahead));
	free_all_stack(argc, numbox, &ahead, &bhead);
	ahead = NULL;
	bhead = NULL;
	return (0);
}
