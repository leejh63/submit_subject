/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_fractal.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/08 18:14:32 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/10 21:56:59 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	diver_check(double *z_x, double *z_y, double c_x, double c_y)
{
	double	t_z_x;

	if ((*z_x) * (*z_x) + (*z_y) * (*z_y) > 4)
		return (1);
	t_z_x = *z_x;
	*z_x = (t_z_x * t_z_x - (*z_y) * (*z_y) + c_x);
	*z_y = (2 * t_z_x * (*z_y) + c_y);
	return (0);
}

int	check_mandel(double c_x, double c_y, t_num *nums)
{
	double	z_x;
	double	z_y;
	int		repeat;

	z_x = 0;
	z_y = 0;
	repeat = -1;
	while (++repeat <= nums -> max_repeat)
	{
		if (diver_check(&z_x, &z_y, c_x, c_y))
			return (repeat);
	}
	return (repeat - 1);
}

int	check_julia(double z_xi, double z_yi, t_num *nums)
{
	double	z_x;
	double	z_y;
	int		repeat;

	z_x = z_xi;
	z_y = z_yi;
	repeat = -1;
	while (++repeat <= nums -> max_repeat)
	{
		if (diver_check(&z_x, &z_y, nums -> c_xi, nums -> c_yi))
			return (repeat);
	}
	return (repeat - 1);
}
