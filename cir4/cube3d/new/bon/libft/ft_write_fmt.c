/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_write_fmt.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/04 16:43:25 by kkeum             #+#    #+#             */
/*   Updated: 2025/05/04 18:43:45 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_write_fmt(t_format format, va_list arg_ptr)
{
	static const t_format	default_fmt = {0, 0, 0, 0, 0, 0, 0, 0, -1};

	if (format.error)
		return (-1);
	if (format.type_spec == 'c')
		return (ft_putc(format, (char)va_arg(arg_ptr, int)));
	else if (format.type_spec == 's')
		return (ft_puts(format, va_arg(arg_ptr, char *)));
	else if (format.type_spec == 'p')
		return (ft_putp(format, (unsigned long long)va_arg(arg_ptr, void *)));
	else if (format.type_spec == 'd' || format.type_spec == 'i')
		return (ft_putd(format, va_arg(arg_ptr, int)));
	else if (format.type_spec == 'u')
		return (ft_putu(format, va_arg(arg_ptr, unsigned int)));
	else if (format.type_spec == 'x' || format.type_spec == 'X')
		return (ft_putx(format, va_arg(arg_ptr, unsigned int)));
	else if (format.type_spec == '%')
		return (ft_putc(default_fmt, '%'));
	return (ft_putc(default_fmt, '%'));
}
