/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mand_arg_check.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/10 21:45:45 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/10 22:01:48 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	int_check(char *str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (!ft_isdigit(str[i]))
			return (1);
		i++;
	}
	if (!i)
		return (1);
	return (0);
}

int	mand_arg_check(char **argv, int max_width, int max_height)
{
	if (type_check(argv[1], 'm'))
		return (err_num(2));
	if (check_window_size(argv, max_width, max_height))
		return (err_num(3));
	return (0);
}
