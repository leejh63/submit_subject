/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_func.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/10 13:03:31 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/10 22:01:13 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	ft_isdigit(int str)
{
	unsigned char	unstr;

	if (str < 0 || str > 255)
		return (0);
	unstr = (unsigned char)str;
	return (unstr >= '0' && unstr <= '9');
}

int	ft_atoi(const char *nptr, int *result)
{
	long long	num;
	int			check;
	int			i;

	num = 0;
	i = 0;
	while (nptr[i])
	{
		check = (nptr[i] - '0');
		if (num > (2147483647 - check) / 10)
			return (1);
		num = num * 10 + (nptr[i] - '0');
		i++;
	}
	*result = (int)num;
	return (0);
}

int	real_front(long double *num, const char *nptr, int *digit, int *sign)
{
	double	check;
	int		i;

	i = 0;
	*sign = 1;
	if (!ft_isdigit(nptr[i]))
	{
		*sign = -1;
		i++;
	}
	while (ft_isdigit(nptr[i]))
	{
		if (++(*digit) > 17)
			return (-1);
		check = (nptr[i] - '0');
		if (*num > (1.7976931348623157e+308 - check) / 10)
			return (-1);
		*num = (*num) * 10.0 + check;
		i++;
	}
	if (nptr[i] != '.')
		return (-2);
	return (i);
}

unsigned long long	real_rear(long double *num, const char *npt, int *dt, int i)
{
	unsigned long long	ten;
	double				check;

	ten = 10;
	while (ft_isdigit(npt[i]))
	{
		if (++(*dt) > 17)
			return (0);
		check = (double)(npt[i] - '0') / ten;
		if (*num > 1.7976931348623157e+308 - check)
			return (0);
		*num = (*num) + check;
		i++;
		ten *= 10;
	}
	return (ten / 10);
}

int	ft_atof(const char *nptr, double *result)
{
	long double			num;
	unsigned long long	ten;
	int					digit;
	int					sign;
	int					i;

	if (float_check(nptr))
		return (1);
	num = 0;
	digit = 0;
	i = real_front(&num, nptr, &digit, &sign);
	if (i == -1)
		return (1);
	else if (i == -2)
	{
		*result = (double)(sign * num);
		return (0);
	}
	ten = real_rear(&num, nptr, &digit, ++i);
	if (!ten)
		return (1);
	*result = (sign * round(num * ten) / ten);
	return (0);
}
