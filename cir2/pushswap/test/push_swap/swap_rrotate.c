/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   swap_rrotate.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 19:02:01 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 15:06:52 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

void	rrotate(t_stack **head, char ab)
{
	t_stack	*det;

	if (!*head)
		return ;
	det = det_b(head);
	add_f(head, det);
	ft_printf("rr%c\n", ab);
}

void	rra(t_stack **head)
{
	rrotate(head, 'a');
}

void	rrb(t_stack **head)
{
	rrotate(head, 'b');
}

void	rrr(t_stack **ahead, t_stack **bhead)
{
	rrotate(ahead, 'a');
	rrotate(bhead, 'b');
}
