/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 13:49:52 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/03 15:15:17 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static size_t	ft_isinset(const char *set, const char schar)
{
	size_t	i;

	i = 0;
	while (set[i])
	{
		if (set[i] == schar)
			return (1);
		i++;
	}
	return (0);
}

char	*ft_strtrim(const char *s1, const char *set)
{
	size_t	front;
	size_t	back;
	char	*newstr;

	if (!s1)
		return (NULL);
	if (!set)
		return (ft_strdup(s1));
	front = 0;
	back = ft_strlen(s1);
	while (s1[front] && ft_isinset(set, s1[front]))
		front++;
	while (front < back && ft_isinset(set, s1[back - 1]))
		back--;
	newstr = malloc(sizeof(char) * (back - front + 1));
	if (!newstr)
		return (NULL);
	ft_strlcpy(newstr, &s1[front], back - front + 1);
	return (newstr);
}
