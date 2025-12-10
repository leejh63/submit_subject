/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putnbr_fd.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 13:19:09 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/04 13:44:11 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_putnbr_fd(int n, int fd)
{
	int		num;
	int		mod;
	char	numchar;

	num = n / 10;
	mod = n % 10;
	if (num)
		ft_putnbr_fd(num, fd);
	if (mod < 0)
	{
		if (!num)
			write(fd, "-", 1);
		mod *= -1;
	}
	numchar = mod + '0';
	write(fd, &numchar, 1);
}
