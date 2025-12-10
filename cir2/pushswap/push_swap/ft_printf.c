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
