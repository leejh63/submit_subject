/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putu.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/04 16:43:18 by kkeum             #+#    #+#             */
/*   Updated: 2025/05/04 18:43:38 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_putu(t_format fmt, unsigned int arg)
{
	char	number[20];
	int		pad[3];
	int		size;

	size = 0;
	ft_ntoa(number, arg, 10);
	if (fmt.precision == 0 && arg == 0)
		number[0] = '\0';
	size = ft_strlen(number);
	pad[1] = get_zero_pad(fmt, size, 0);
	size += pad[1];
	pad[0] = get_front_pad(fmt, size);
	pad[2] = get_rear_pad(fmt, size);
	ft_put_padding(' ', pad[0]);
	ft_put_padding('0', pad[1]);
	ft_putstr(number);
	ft_put_padding(' ', pad[2]);
	size += pad[0] + pad[2];
	return (size);
}
