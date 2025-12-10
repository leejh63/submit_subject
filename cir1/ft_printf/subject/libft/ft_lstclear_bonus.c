/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstclear.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 12:40:00 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/07 12:55:03 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_lstclear(t_list **lst, void (*del)(void *))
{
	t_list	*check;
	t_list	*temp;

	if (!del || !lst)
		return ;
	check = *lst;
	while (check)
	{
		temp = check -> next;
		ft_lstdelone(check, del);
		check = temp;
	}
	*lst = NULL;
}
