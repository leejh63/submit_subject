/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_puts.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/04 16:43:15 by kkeum             #+#    #+#             */
/*   Updated: 2025/05/04 18:43:36 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_puts(t_format fmt, char *arg)
{
	int	str_len;
	int	padding_len;

	if (!arg)
	{
		arg = "(null)";
		if (fmt.precision >= 0 && fmt.precision < 6)
			arg = "";
	}
	if (fmt.precision >= 0)
		str_len = min(ft_strlen(arg), fmt.precision);
	else
		str_len = ft_strlen(arg);
	padding_len = fmt.width - str_len;
	padding_len *= (padding_len > 0);
	if (!fmt.minus)
		ft_put_padding(' ', padding_len);
	write(1, arg, str_len);
	if (fmt.minus)
		ft_put_padding(' ', padding_len);
	return (str_len + padding_len);
}
