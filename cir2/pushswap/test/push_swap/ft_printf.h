/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/09 15:21:37 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/04/09 15:21:41 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PRINTF_H
# define FT_PRINTF_H

# include "libft.h"
# include <stdarg.h>

void	ft_pbase(unsigned long long n, int *i, int basenum, int c);
void	ft_pputnbr(int n, int *i);
int		ft_printf(const char *string, ...);
int		ft_print_chr(va_list *args);
int		ft_print_str(va_list *args);
int		ft_print_ptr(va_list *args);
int		ft_print_dec(va_list *args);
int		ft_print_int(va_list *args);
int		ft_print_uit(va_list *args);
int		ft_print_hex(va_list *args);
int		ft_print_lex(va_list *args);
int		ft_print_check(const char *string, va_list *args);

#endif
