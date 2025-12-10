/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putx.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/04 16:43:21 by kkeum             #+#    #+#             */
/*   Updated: 2025/05/04 18:43:42 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

static void	upper(char *str)
{
	while (*str)
	{
		*str = ft_toupper(*str);
		str++;
	}
}

int	ft_putx(t_format fmt, unsigned int arg)
{
	char	*hash;
	char	number[20];
	int		pad[3];
	int		size;

	size = 0;
	hash = tenary(fmt.hash, tenary(fmt.type_spec == 'x', "0x", "0X"), "");
	hash = tenary(arg, hash, "");
	ft_ntoa(number, arg, 16);
	if (fmt.precision == 0 && arg == 0)
		number[0] = '\0';
	if (fmt.type_spec == 'X')
		upper(number);
	size = ft_strlen(number);
	pad[1] = get_zero_pad(fmt, size, 0);
	size += pad[1];
	pad[0] = get_front_pad(fmt, size);
	pad[2] = get_rear_pad(fmt, size);
	ft_put_padding(' ', pad[0]);
	ft_putstr(hash);
	ft_put_padding('0', pad[1]);
	ft_putstr(number);
	ft_put_padding(' ', pad[2]);
	size += ft_strlen(hash) + pad[0] + pad[2];
	return (size);
}
