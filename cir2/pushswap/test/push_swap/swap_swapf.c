/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   swap_swapf.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 18:58:34 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 15:06:55 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

void	swap(t_stack **head, char ab)
{
	long long	num;

	num = (*head)->num;
	(*head)->num = (*head)->next->num;
	(*head)->next ->num = num;
	ft_printf("s%c\n", ab);
}

void	sa(t_stack **head)
{
	swap(head, 'a');
}

void	sb(t_stack **head)
{
	swap(head, 'b');
}

void	ss(t_stack **ahead, t_stack **bhead)
{
	swap(ahead, 'a');
	swap(bhead, 'b');
}
