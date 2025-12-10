/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_parse_fmt.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/04 16:40:44 by kkeum             #+#    #+#             */
/*   Updated: 2025/05/04 18:43:26 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

static int	cinstr(char c, char *str)
{
	if (!str)
		return (0);
	while (*str)
	{
		if (*str++ == c)
			return (1);
	}
	return (0);
}

static int	is_flag(char c)
{
	return (cinstr(c, "0-+# "));
}

static int	is_valid_specifier(char c)
{
	return (cinstr(c, "cspdiuxX%"));
}

static void	set_flag(t_format *format, char c)
{
	if (c == '0')
		format->zero = 1;
	else if (c == '-')
		format->minus = 1;
	else if (c == '+')
		format->plus = 1;
	else if (c == '#')
		format->hash = 1;
	else if (c == ' ')
		format->blank = 1;
}

int	ft_parse_fmt(char **str, va_list arg_ptr)
{
	static const t_format	default_fmt = {0, 0, 0, 0, 0, 0, 0, 0, -1};
	t_format				format;
	int						i;

	if (*(*str)++ != '%')
		return (0);
	i = 0;
	format = default_fmt;
	while (is_flag(*(*str + i)))
		set_flag(&format, *(*str + i++));
	if (ft_isdigit(*(*str + i)))
		format.width = ft_atoi(*str + i);
	while (ft_isdigit(*(*str + i)))
		i++;
	if (*(*str + i) == '.')
		format.precision = ft_atoi(*str + ++i);
	while (ft_isdigit(*(*str + i)))
		i++;
	format.type_spec = *(*str + i);
	if (is_valid_specifier(format.type_spec))
		*str = *str + ++i;
	if (format.width < -1 || format.precision < -1)
		format.error = 1;
	return (ft_write_fmt(format, arg_ptr));
}
