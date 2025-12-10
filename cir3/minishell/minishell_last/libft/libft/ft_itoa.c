/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 14:55:42 by kkeum             #+#    #+#             */
/*   Updated: 2025/04/07 20:36:59 by kkeum            ###   ########.fr       */
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

char	*ft_itoa(int n)
{
	char		digits[11];
	char		*str;
	int			i;
	int			j;
	long long	nll;

	if (n == 0)
		return (ft_strdup("0"));
	nll = (long long)n;
	if (nll < 0)
		nll *= -1;
	i = reverse_itoa(nll, digits);
	if (n < 0)
		digits[i++] = '-';
	str = (char *)malloc(i + 1);
	if (!str)
		return (0);
	str[i] = '\0';
	j = -1;
	while (++j < i)
		str[j] = digits[i - j - 1];
	return (str);
}
/*
#include <stdio.h>
int main(void)
{
	printf("%s\n", ft_itoa(-2147483648));
	printf("%s\n", ft_itoa(-1));
	printf("%s\n", ft_itoa(0));
	printf("%s\n", ft_itoa(1));
	return (0);
}*/
