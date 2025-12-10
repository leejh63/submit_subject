/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoi.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaeholee <jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/02 17:32:35 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/10 14:59:48 by jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

const char	*ft_skipspace(const char *nptr)
{
	while (*nptr == ' ' || (*nptr >= '\t' && *nptr <= '\r'))
		nptr++;
	return (nptr);
}

int	ft_atoi(const char *nptr)
{
	unsigned int	minus;
	int				num;

	if (!nptr)
		return (0);
	minus = 1;
	nptr = ft_skipspace(nptr);
	if (*nptr == '-' || *nptr == '+')
	{
		if (*nptr == '-')
			minus = -1;
		nptr++;
	}
	num = 0;
	while (ft_isdigit((int)*nptr))
	{
		num = num * 10 + (*nptr - '0');
		nptr++;
	}
	return (minus * num);
}
