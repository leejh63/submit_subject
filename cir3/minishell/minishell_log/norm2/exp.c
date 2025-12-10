/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   find_path.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 16:29:49 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/11 16:29:50 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	fexp_lstadd_back(t_exp **lst, t_exp *new)
{
	t_exp	*cursor;

	if (lst == 0 || new == 0)
		return ;
	if (*lst == 0)
		*lst = new;
	else
	{
		cursor = *lst;
		while (cursor->next)
			cursor = cursor->next;
		cursor->next = new;
	}
}

t_exp	*fexp_lstlast(t_exp *lst)
{
	t_exp	*cursor;

	if (lst == NULL)
		return (NULL);
	cursor = lst;
	while (cursor->next)
		cursor = cursor->next;
	return (cursor);
}
