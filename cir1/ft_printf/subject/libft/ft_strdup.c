/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strdup.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/02 19:02:17 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/02 19:20:50 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strdup(const char *s)
{
	char			*dup;
	unsigned int	orilen;

	orilen = ft_strlen(s);
	dup = malloc(sizeof(char) * (++orilen));
	if (!dup)
		return (0);
	return ((char *)ft_memcpy((void *)dup, (void *)s, orilen));
}
