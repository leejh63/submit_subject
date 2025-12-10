/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jaeholee <jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/08 11:27:40 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/09 15:19:16 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_printf(const char *string, ...)
{
	va_list	args;
	int		i;
	int		m;

	i = 0;
	//string, args 이 NULL들어올때 처리 필요해 보임!
	// 또한 가변인자를 꺼내오는 횟수에 제한을 두는것이 좋아보인다.
	va_start(args, string);
	while (*string)
	{
		if (*string == '%')
		{
			string++;
			m = ft_print_check(string, &args);
			if (m < 0)
			{
				va_end(args);
				return (-1);
			}
			i += m;
		}
		else
			i += write(1, string, 1);
		string++;
	}
	va_end(args);
	return (i);
}
