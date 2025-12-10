/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/04 16:43:07 by kkeum             #+#    #+#             */
/*   Updated: 2025/05/04 18:43:28 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_printf(const char *str, ...)
{
	va_list	arg_ptr;
	int		byte_size;
	int		i;

	if (str == 0)
		return (-1);
	byte_size = 0;
	va_start(arg_ptr, str);
	while (*str)
	{
		byte_size += put_until_fmt(&str);
		if (*str == 0)
			break ;
		i = ft_parse_fmt((char **)&str, arg_ptr);
		if (i == -1)
			return (-1);
		byte_size += i;
	}
	va_end(arg_ptr);
	return (byte_size);
}
/*
#include <stdio.h>
int main (void)
{
	int	asdf;

	printf("%c\t%s\t%p\t%d\t%i\t%u\t%x\t%X\t%%", 'a', "helloworld", \
		 &asdf, 123, 456, 3333333333u, 0x123, 0x456);
	printf("\n");
	fflush(stdout);
	ft_printf("%c\t%s\t%p\t%d\t%i\t%u\t%x\t%X\t%%", 'a', "helloworld", \
		 &asdf, 123, 456, 3333333333u, 0x123, 0x456);
	 asdf = ft_printf("%% %%");
	 ft_printf("%s\n", "helloworld");
	 ft_printf("%p\n", &asdf);
	 ft_printf("%d\n", 123);
	 ft_printf("%i\n", 456);
	 ft_printf("%u\n", 3333333333u);
	 ft_printf("%x\n", 0x123a);
	 ft_printf("%X\n", 0x456d);
	 ft_printf("%%\n");
	 printf("\n\n%d\n", asdf);
}*/
