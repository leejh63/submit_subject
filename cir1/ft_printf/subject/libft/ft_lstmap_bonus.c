/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstmap.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 13:08:59 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/07 13:46:08 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

t_list	*ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *))
{
	t_list	*flist;
	t_list	*newnode;
	void	*fvalue;

	if (!lst || !f || !del)
		return (NULL);
	flist = NULL;
	while (lst)
	{
		fvalue = f(lst -> content);
		newnode = ft_lstnew(fvalue);
		if (!newnode)
		{
			ft_lstclear(&flist, del);
			return (flist);
		}
		ft_lstadd_back(&flist, newnode);
		lst = lst -> next;
	}
	return (flist);
}
