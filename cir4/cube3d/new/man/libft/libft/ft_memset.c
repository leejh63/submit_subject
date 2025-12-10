/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memset.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 14:31:09 by kkeum             #+#    #+#             */
/*   Updated: 2025/04/07 13:57:22 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memset(void *s, int c, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		*((unsigned char *)s + i) = (unsigned char)c;
		i++;
	}
	return (s);
}

/*
#include <stdio.h>

int	main(void)
{
	char	str[20] = "abcdefghijklmnopqrs";
	char	*ptr;

	ptr = ft_memchr(str, 'c', 20);
	printf("str : %s\n", ptr);
	return (0);
}*/
