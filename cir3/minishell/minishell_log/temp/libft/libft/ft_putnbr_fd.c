/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putnbr_fd.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 15:58:46 by kkeum             #+#    #+#             */
/*   Updated: 2025/04/07 20:24:07 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	reverse_itoa(long long nll, char *arr)
{
	int	i;

	i = 0;
	while (nll)
	{
		arr[i] = '0' + (nll % 10ll);
		nll /= 10ll;
		i++;
	}
	return (i);
}

static void	reverse_str(char *str, size_t len)
{
	size_t	i;
	char	c;

	if (len <= 1)
		return ;
	i = 0;
	len--;
	while (i < len)
	{
		c = str[i];
		str[i] = str[len];
		str[len] = c;
		i++;
		len--;
	}
}

static void	itoa(char *digits, int n)
{
	int			i;
	long long	nll;

	if (n == 0)
	{
		digits[0] = '0';
		digits[1] = '\0';
		return ;
	}
	nll = (long long)n;
	if (nll < 0)
		nll *= -1;
	i = reverse_itoa(nll, digits);
	if (n < 0)
		digits[i++] = '-';
	digits[i] = '\0';
	reverse_str(digits, i);
}

void	ft_putnbr_fd(int n, int fd)
{
	char	digits[12];

	itoa(digits, n);
	ft_putstr_fd(digits, fd);
}
/*
int main(void)
{
	ft_putnbr_fd(0, 1);
	ft_putchar_fd('\n', 1);
	ft_putnbr_fd(1, 1);
	ft_putchar_fd('\n', 1);
	ft_putnbr_fd(10, 1);
	ft_putchar_fd('\n', 1);
}*/
