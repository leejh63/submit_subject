/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstadd_back.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 12:02:20 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/07 12:25:02 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_lstadd_back(t_list **lst, t_list *new)
{
	t_list	*check;

	if (!new)
		return ;
	if (!(*lst))
	{
		*lst = new;
		return ;
	}
	check = *lst;
	while (check -> next)
		check = check -> next;
	check -> next = new;
}
