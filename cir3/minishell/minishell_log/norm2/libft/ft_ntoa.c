/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ntoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/04 16:40:39 by kkeum             #+#    #+#             */
/*   Updated: 2025/05/04 18:43:24 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"
#define BASE "0123456789abcdef"

static size_t	reverse_ntoa(unsigned long long n, char *arr, int base_len)
{
	int	i;

	i = 0;
	while (n)
	{
		arr[i] = BASE[n % base_len];
		n /= base_len;
		i++;
	}
	return (i);
}

void	ft_ntoa(char *digits, unsigned long long n, int base_len)
{
	size_t	i;

	if (base_len < 2)
	{
		ft_strlcpy(digits, "", 1);
		return ;
	}
	if (n == 0)
	{
		ft_strlcpy(digits, "0", 2);
		return ;
	}
	i = reverse_ntoa(n, digits, base_len);
	digits[i] = '\0';
	reverse_str(digits, i);
}
