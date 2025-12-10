/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstmap_bonus.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaeholee <jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 13:08:59 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/08 17:52:04 by jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

t_list	*ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *))
{
	t_list	*flist;
	t_list	*newnode;
	void	*fvalue;

	if (!lst || !f)
		return (NULL);
	flist = NULL;
	while (lst)
	{
		fvalue = f(lst -> content);
		newnode = ft_lstnew(fvalue);
		if (!newnode)
		{
			if (del)
				del(fvalue);
			ft_lstclear(&flist, del);
			return (flist);
		}
		ft_lstadd_back(&flist, newnode);
		lst = lst -> next;
	}
	return (flist);
}
