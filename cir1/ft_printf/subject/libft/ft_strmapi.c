/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strmapi.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 11:33:21 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/04 12:07:58 by Jaeholee         ###   ########.fr       */
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
