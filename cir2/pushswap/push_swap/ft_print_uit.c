/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_uit.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 11:29:48 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/10 11:29:53 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_print_uit(va_list *args)
{
	unsigned int	n;
	int				i;

	n = va_arg(*args, unsigned int);
	i = 0;
	ft_pbase(n, &i, 10, 0);
	return (i);
}
