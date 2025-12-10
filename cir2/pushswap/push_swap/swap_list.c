/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   swap_list.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 18:44:13 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 15:06:31 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

t_stack	*new_num(long long num)
{
	t_stack	*new;

	new = malloc(sizeof(t_stack));
	if (!new)
		return (NULL);
	new -> next = NULL;
	new -> num = num;
	return (new);
}

void	add_f(t_stack **head, t_stack *new)
{
	if (!*head)
	{
		new -> next = NULL;
		*head = new;
		return ;
	}
	new -> next = *head;
	*head = new;
}

void	add_b(t_stack **head, t_stack *new)
{
	t_stack	*back;

	if (!*head)
	{
		add_f(head, new);
		return ;
	}
	back = (*head);
	while (back -> next)
		back = back -> next;
	back -> next = new;
	new -> next = NULL;
	return ;
}

t_stack	*det_f(t_stack **head)
{
	t_stack	*def;

	def = (*head);
	if (def -> next)
		(*head) = (*head)-> next;
	else
		(*head) = NULL;
	return (def);
}

t_stack	*det_b(t_stack **head)
{
	t_stack	*deb;
	t_stack	*tmp;

	if (!*head)
		return (NULL);
	deb = (*head);
	if (!(deb -> next))
	{
		*head = NULL;
		return (deb);
	}
	while (deb -> next)
	{
		if (!(deb -> next -> next))
			tmp = deb;
		deb = deb -> next;
	}
	tmp -> next = NULL;
	return (deb);
}
