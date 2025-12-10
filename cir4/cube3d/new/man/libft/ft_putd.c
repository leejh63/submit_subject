/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putd.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/04 18:43:31 by kkeum             #+#    #+#             */
/*   Updated: 2025/05/04 18:43:32 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_putd(t_format fmt, int arg)
{
	char	*sign;
	char	number[11];
	int		pad[3];
	int		size;

	sign = tenary(fmt.plus, "+", tenary(fmt.blank, " ", ""));
	sign = tenary(arg < 0, "-", sign);
	ft_ntoa(number, ft_abs(arg), 10);
	if (fmt.precision == 0 && arg == 0)
		number[0] = '\0';
	pad[1] = get_zero_pad(fmt, ft_strlen(number), ft_strlen(sign));
	size = ft_strlen(sign) + pad[1] + ft_strlen(number);
	pad[0] = get_front_pad(fmt, size);
	pad[2] = get_rear_pad(fmt, size);
	ft_put_padding(' ', pad[0]);
	ft_putstr(sign);
	ft_put_padding('0', pad[1]);
	ft_putstr(number);
	ft_put_padding(' ', pad[2]);
	size += pad[0] + pad[2];
	return (size);
}
