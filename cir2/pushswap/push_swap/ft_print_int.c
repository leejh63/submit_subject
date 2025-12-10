/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_int.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 11:28:39 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/10 11:28:45 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_print_int(va_list *args)
{
	int	ft_int;
	int	i;

	i = 0;
	ft_int = va_arg(*args, int);
	ft_pputnbr(ft_int, &i);
	return (i);
}
