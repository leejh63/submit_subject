/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_ptr.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 11:26:40 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/10 11:26:48 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_print_ptr(va_list *args)
{
	void	*n;
	int		i;

	n = va_arg(*args, void *);
	i = 0;
	if (!n)
		i += write(1, "(nil)", 5);
	else
	{
		i += write(1, "0x", 2);
		ft_pbase((unsigned long long)n, &i, 16, 0);
	}
	return (i);
}
