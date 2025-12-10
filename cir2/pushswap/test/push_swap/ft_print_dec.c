/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_dec.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 11:27:54 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/10 11:27:58 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_print_dec(va_list *args)
{
	int	dec;
	int	i;

	i = 0;
	dec = va_arg(*args, int);
	ft_pputnbr(dec, &i);
	return (i);
}
