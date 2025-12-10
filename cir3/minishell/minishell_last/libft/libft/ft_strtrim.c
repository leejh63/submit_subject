/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/06 16:13:57 by kkeum             #+#    #+#             */
/*   Updated: 2025/04/07 20:36:59 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	ft_cinset(char c, char *set)
{
	while (*set)
	{
		if (*set == c)
			return (1);
		set++;
	}
	return (0);
}

char	*ft_strtrim(char const *s1, char const *set)
{
	size_t	s;
	size_t	e;
	char	*ptr;

	s = 0;
	e = ft_strlen((char *)s1);
	while (ft_cinset(*(s1 + s), (char *)set))
		s++;
	while (ft_cinset(*(s1 + e - 1), (char *)set) && e > s)
		e--;
	ptr = (char *)malloc(e - s + 1);
	if (!ptr)
		return (0);
	ft_strlcpy(ptr, s1 + s, e - s + 1);
	return (ptr);
}
