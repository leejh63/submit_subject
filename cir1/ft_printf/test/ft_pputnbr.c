/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_pputnbr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 11:21:04 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/10 11:21:35 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

void	ft_pputnbr(int n, int *i)
{
	int		num;
	int		mod;
	char	numchar;

	num = n / 10;
	mod = n % 10;
	if (num)
		ft_pputnbr(num, i);
	if (mod < 0)
	{
		if (!num)
			*i += write(1, "-", 1);
		mod *= -1;
	}
	numchar = mod + '0';
	*i += write(1, &numchar, 1);
}
