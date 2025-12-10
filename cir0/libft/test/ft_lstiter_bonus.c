/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstiter_bonus.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaeholee <jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 13:03:47 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/08 17:53:58 by jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_lstiter(t_list *lst, void (*f)(void *))
{
	t_list	*fvalue;

	if (!lst || !f)
		return ;
	fvalue = lst;
	while (fvalue)
	{
		f(fvalue -> content);
		fvalue = fvalue -> next;
	}
}
