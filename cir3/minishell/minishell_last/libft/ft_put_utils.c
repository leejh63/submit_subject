/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_put_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/04 16:43:20 by kkeum             #+#    #+#             */
/*   Updated: 2025/05/04 18:43:40 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	get_front_pad(t_format fmt, int arg_len)
{
	if (fmt.minus)
		return (0);
	if (fmt.zero && fmt.precision == -1)
		return (0);
	return (max(fmt.width - arg_len, 0));
}

int	get_zero_pad(t_format fmt, int arg_len, int sign_len)
{
	if (!fmt.minus && fmt.zero && fmt.precision == -1)
		return (max(fmt.width - sign_len - arg_len, 0));
	return (max(fmt.precision - arg_len, 0));
}

int	get_rear_pad(t_format fmt, int arg_len)
{
	if (!fmt.minus)
		return (0);
	if (fmt.precision == -1)
		return (max(fmt.width - arg_len, 0));
	return (max(fmt.width - max(arg_len, fmt.precision), 0));
}

void	ft_put_padding(char c, size_t times)
{
	while (times > 0)
	{
		write(1, &c, 1);
		times --;
	}
}

int	ft_putstr(char *str)
{
	int	size;

	size = ft_strlen(str);
	write(1, str, size);
	return (size);
}
