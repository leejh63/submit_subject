/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/08 13:06:05 by kkeum             #+#    #+#             */
/*   Updated: 2025/05/04 19:46:17 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PRINTF_H
# define FT_PRINTF_H

# include "libft/libft.h"
# include <stdarg.h>

typedef struct s_format {
	int		error;
	int		type_spec;
	int		minus;
	int		zero;
	int		plus;
	int		blank;
	int		hash;
	int		width;
	int		precision;
}	t_format;
typedef unsigned long long	t_ull;

int		ft_printf(const char *str, ...);
int		ft_parse_fmt(char **str, va_list arg_ptr);
int		ft_write_fmt(t_format format, va_list arg_ptr);
int		put_until_fmt(char const **str);
void	ft_ntoa(char *digits, unsigned long long n, int base_len);
void	ft_put_padding(char c, size_t times);
int		ft_putstr(char *str);
int		ft_putc(t_format fmt, char arg);
int		ft_putp(t_format fmt, unsigned long long arg);
int		ft_puts(t_format fmt, char *arg);
int		ft_putd(t_format fmt, int arg);
int		ft_putu(t_format fmt, unsigned int arg);
int		ft_putx(t_format fmt, unsigned int arg);
int		get_front_pad(t_format fmt, int arg_len);
int		get_zero_pad(t_format fmt, int arg_len, int sign_len);
int		get_rear_pad(t_format fmt, int arg_len);
char	*tenary(int condition, char *s1, char *s2);
t_ull	ft_abs(int n);
void	reverse_str(char *str, size_t len);
int		max(int n1, int n2);
int		min(int n1, int n2);

#endif
