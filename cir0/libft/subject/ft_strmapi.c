/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strmapi.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaeholee <jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 11:33:21 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/10 15:10:49 by jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strmapi(const char *s, char (*f)(unsigned int, char))
{
	unsigned int	i;
	char			*fstring;
	size_t			slen;

	if (!s)
		return (NULL);
	i = 0;
	slen = ft_strlen(s);
	fstring = malloc(sizeof (char) * (slen + 1));
	if (!fstring)
		return (NULL);
	fstring[slen] = '\0';
	while (s[i])
	{
		fstring[i] = f(i, s[i]);
		i++;
	}
	return (fstring);
}
