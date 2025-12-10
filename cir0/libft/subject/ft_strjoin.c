/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strjoin.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaeholee <jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 13:21:28 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/10 15:11:53 by jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strjoin(char const *s1, char const *s2)
{
	size_t	s1len;
	size_t	s2len;
	char	*newstr;

	if (!s1 || !s2)
		return (NULL);
	s1len = ft_strlen(s1);
	s2len = ft_strlen(s2);
	newstr = malloc(sizeof(char) * (s1len + s2len + 1));
	if (!newstr)
		return (NULL);
	ft_strlcpy(newstr, s1, s1len + 1);
	ft_strlcat(newstr, s2, s1len + s2len + 1);
	return (newstr);
}
