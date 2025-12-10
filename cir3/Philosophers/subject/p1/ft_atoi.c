/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_atoi.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/17 14:37:41 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/17 14:37:42 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

int	ft_isdigit(int str)
{
	unsigned char	unstr;

	if (str < 0 || str > 255)
		return (1);
	unstr = (unsigned char)str;
	return (unstr < '0' || unstr > '9');
}

int	ft_atoi(long long *result, char *asc)
{
	int			sign;
	int			i;
	long long	num;

	i = 0;
	sign = 1;
	num = 0;
	if (ft_isdigit(asc[i]))
	{
		if (asc[i] == '-' && asc[++i])
			sign = -1;
		else
			return (1);
	}
	while (ft_isdigit(asc[i]) == 0)
	{
		num = num * 10 + (asc[i] - '0');
		i++;
	}
	if (asc[i] || (sign * num) > 2147483647 || (sign * num) < -2147483648)
		return (1);
	*result = sign * num;
	return (0);
}
