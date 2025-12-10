/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   swap_pushf.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 18:58:49 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 15:06:37 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

void	push(t_stack **ahead, t_stack **bhead, char ab)
{
	t_stack	*def;

	if (ab == 'b')
	{
		if (!ahead)
			return ;
		def = det_f(ahead);
		add_f(bhead, def);
	}
	else
	{
		if (!bhead)
			return ;
		def = det_f(bhead);
		add_f(ahead, def);
	}
	ft_printf("p%c\n", ab);
}

void	pa(t_stack **ahead, t_stack **bhead)
{
	push(ahead, bhead, 'a');
}

void	pb(t_stack **ahead, t_stack **bhead)
{
	push(ahead, bhead, 'b');
}
