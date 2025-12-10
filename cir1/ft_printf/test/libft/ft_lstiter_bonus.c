/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstiter.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 13:03:47 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/07 13:06:48 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_lstiter(t_list *lst, void (*f)(void *))
{
	t_list	*appl;

	if (!lst || !f)
		return ;
	appl = lst;
	while (appl)
	{
		f(appl -> content);
		appl = appl -> next;
	}
}
