/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strnstr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 14:35:39 by kkeum             #+#    #+#             */
/*   Updated: 2025/04/06 21:31:38 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strnstr(const char *src, const char *target, size_t len)
{
	size_t	i;
	size_t	j;

	if (target[0] == '\0')
		return ((char *)src);
	i = 0;
	while (src[i])
	{
		j = 0;
		while (1)
		{
			if (target[j] == '\0')
				return ((char *)src + i);
			if (src[i + j] != target[j])
				break ;
			if (i + j >= len)
				return (0);
			j++;
		}
		i++;
		if (i > len)
			break ;
	}
	return (0);
}
/*
#include <stdio.h>

int main(void)
{
	char *s1 = "";
	char *s2 = "ll";
	printf("%s\n%s\n%s\n",s1, s2, ft_strnstr(s1, s2, -1));
	return (0);
}
*/