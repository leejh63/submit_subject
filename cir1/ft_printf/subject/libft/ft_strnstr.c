/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strnstr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/02 16:58:03 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/02 17:20:55 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strnstr(const char *bi, const char *li, size_t len)
{
	size_t	llen;

	llen = ft_strlen(li);
	if (!llen)
		return ((char *)bi);
	while (*bi && len >= llen)
	{
		if (!ft_memcmp(bi, li, llen))
			return ((char *)bi);
		bi++;
		len--;
	}
	return (0);
}
