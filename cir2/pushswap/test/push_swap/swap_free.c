/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   swap_free.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 11:14:39 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/01 15:06:20 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

static void	free_all(char **words)
{
	int	i;

	i = 0;
	while (words[i])
	{
		free(words[i]);
		i++;
	}
	free(words);
}

void	free_stack(t_stack **head)
{
	t_stack	*tmp;

	if (*head)
	{
		while (*head)
		{
			tmp = (*head)->next;
			free(*head);
			*head = tmp;
		}
		*head = NULL;
	}
}

int	free_all_stack(int argc, char **numbox, t_stack **ahead, t_stack **bhead)
{
	if (argc == 2)
		free_all(numbox);
	free_stack(ahead);
	free_stack(bhead);
	return (0);
}
