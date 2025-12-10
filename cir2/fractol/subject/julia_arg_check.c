/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   julia_arg_check.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/10 21:41:30 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/10 22:01:26 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	float_check(const char *str)
{
	int	i;

	i = 0;
	if (!ft_isdigit(str[i]))
	{
		if (str[i++] != '-')
			return (1);
	}
	if (!ft_isdigit(str[i]))
		return (1);
	while (ft_isdigit(str[i]))
		i++;
	if (str[i] != '.')
		return (str[i] != '\0');
	else
		i++;
	if (!str[i])
		return (1);
	while (str[i])
	{
		if (!ft_isdigit(str[i]))
			return (1);
		i++;
	}
	return (0);
}

int	julia_arg_check(char **argv, int max_width, int max_height)
{
	double	tmp_f;

	if (type_check(argv[1], 'j'))
		return (err_num(2));
	if (check_window_size(argv, max_width, max_height))
		return (err_num(3));
	if (ft_atof(argv[4], &tmp_f) || ft_atof(argv[5], &tmp_f))
		return (err_num(4));
	return (0);
}
