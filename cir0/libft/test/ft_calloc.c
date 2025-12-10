/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_calloc.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/02 18:25:47 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/02 18:59:18 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_calloc(size_t nmemb, size_t size)
{
	void	*m;
	size_t	multi;

	if (size && (nmemb > (SIZE_MAX / size)))
		return (NULL);
	multi = nmemb * size;
	m = malloc(multi);
	if (!m)
		return (NULL);
	ft_bzero(m, multi);
	return (m);
}
