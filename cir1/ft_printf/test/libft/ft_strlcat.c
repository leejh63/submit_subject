/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlcat.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/01 17:21:39 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/01 18:02:02 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

size_t	ft_strlcat(char *dst, const char *src, size_t size)
{
	size_t	j;
	size_t	dsize;
	size_t	ssize;

	ssize = ft_strlen(src);
	if (!size)
		return (ssize);
	dsize = ft_strlen(dst);
	if (dsize >= size)
		return (size + ssize);
	j = 0;
	while (src[j] && ((dsize + j) < (size - 1)))
	{
		dst[dsize + j] = src[j];
		j++;
	}
	dst[dsize + j] = '\0';
	return (dsize + ssize);
}
