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

static unsigned int	ft_isinset(const char *set, const char schar)
{
	unsigned int	i;

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
	unsigned int	front;
	unsigned int	rear;
	char			*newstr;

	if (!s1)
		return (NULL);
	if (!set)
		return (ft_strdup(""));
	front = 0;
	rear = ft_strlen(s1);
	while (s1[front] && ft_isinset(set, s1[front]))
		front++;
	while (front < rear && ft_isinset(set, s1[rear - 1]))
		rear--;
	newstr = malloc(sizeof(char) * (rear - front + 1));
	if (!newstr)
		return (NULL);
	ft_strlcpy(newstr, &s1[front], rear - front + 1);
	return (newstr);
}
