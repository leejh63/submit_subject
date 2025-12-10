/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 17:16:47 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/03 18:26:24 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_itoa(int n)
{
	char	stack[32];
	char	*string;
	int		i;
	int		sign;

	i = 0;
	sign = 0;
	stack[0] = '\0';
	while (n / 10 != 0)
	{
		stack[++i] = (((n >= 0) - (n < 0))) * (n % 10) + '0';
		n = (n / 10);
	}
	stack[++i] = (((n >= 0) - (n < 0))) * (n % 10) + '0';
	if (n < 0)
		stack[++i] = '-';
	i++;
	string = malloc(sizeof (char) * i);
	if (!string)
		return (NULL);
	while (i--)
		string[sign++] = stack[i];
	return (string);
}
