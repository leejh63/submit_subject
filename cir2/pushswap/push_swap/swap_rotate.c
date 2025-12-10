/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   swap_rotate.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 19:00:36 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 15:06:44 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

void	rotate(t_stack **head, char ab)
{
	t_stack	*det;

	if (!*head)
		return ;
	det = det_f(head);
	add_b(head, det);
	ft_printf("r%c\n", ab);
}

void	ra(t_stack **head)
{
	rotate(head, 'a');
}

void	rb(t_stack **head)
{
	rotate(head, 'b');
}

void	rr(t_stack **ahead, t_stack **bhead)
{
	rotate(ahead, 'a');
	rotate(bhead, 'b');
}
