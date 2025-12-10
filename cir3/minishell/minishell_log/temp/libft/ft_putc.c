/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/04 16:43:09 by kkeum             #+#    #+#             */
/*   Updated: 2025/05/04 18:43:30 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_putc(t_format fmt, char arg)
{
	int	padding_len;

	padding_len = fmt.width - 1;
	padding_len *= (padding_len > 0);
	if (!fmt.minus)
		ft_put_padding(' ', padding_len);
	write(1, &arg, 1);
	if (fmt.minus)
		ft_put_padding(' ', padding_len);
	return (1 + padding_len);
}
