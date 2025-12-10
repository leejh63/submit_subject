/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memmove.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/06 15:07:15 by kkeum             #+#    #+#             */
/*   Updated: 2025/04/06 15:07:43 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memmove(void *dest, const void *src, size_t n)
{
	size_t	i;

	i = 0;
	if (src > dest)
	{
		while (i < n)
		{
			*((unsigned char *)dest + i) = *((unsigned char *) src + i);
			i++;
		}
	}
	else
	{
		while (i < n)
		{
			*((unsigned char *)dest + n - i - 1)
				= *((unsigned char *) src + n - i - 1);
			i++;
		}
	}
	return (dest);
}
