/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_check.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 11:32:45 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/10 11:32:48 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_print_check(const char *string, va_list *args)
{
	int	i;

	i = 0;
	if (*string == 'c')
		i += ft_print_chr(args);
	else if (*string == 's')
		i += ft_print_str(args);
	else if (*string == 'p')
		i += ft_print_ptr(args);
	else if (*string == 'd')
		i += ft_print_dec(args);
	else if (*string == 'i')
		i += ft_print_int(args);
	else if (*string == 'u')
		i += ft_print_uit(args);
	else if (*string == 'x')
		i += ft_print_hex(args);
	else if (*string == 'X')
		i += ft_print_lex(args);
	else if (*string == '%')
		i += write(1, "%", 1);
	else
		i = -1;
	return (i);
}
