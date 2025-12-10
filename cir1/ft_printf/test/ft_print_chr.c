/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_chr.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 11:23:34 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/10 11:23:50 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_print_chr(va_list *args)
{
	unsigned char	chr;

	chr = (unsigned char)va_arg(*args, int);
	write(1, &chr, 1);
	return (1);
}
