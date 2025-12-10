/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   burnship_fractal.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/09 20:32:08 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/09 20:32:09 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	bdiver_check(double *z_x, double *z_y, double c_x, double c_y)
{
	double	t_z_x;

	if ((*z_x) * (*z_x) + (*z_y) * (*z_y) > 4)
		return (1);
	t_z_x = *z_x;
	*z_x = (t_z_x * t_z_x - (*z_y) * (*z_y) + c_x);
	*z_y = (2 * t_z_x * (*z_y) + c_y);
	return (0);
}

int	check_burnship(double c_x, double c_y, t_num *nums)
{
	double	z_x;
	double	z_y;
	int	repeat;

	z_x = 0;
	z_y = 0;
	repeat = -1;
	while (++repeat <= nums -> max_repeat)
	{
		z_x = fabs(z_x);
		z_y = fabs(z_y);
		if (bdiver_check(&z_x, &z_y, c_x, c_y))
			return (repeat);
	}
	return (repeat - 1);
}

int	burnship_fractal(int p_x, int p_y, t_num *nums)
{
	double	z_x;
	double	z_y;
	int		repeat;

	z_x = convert_x(p_x, nums);
	z_y = convert_y(p_y, nums);
	repeat = check_burnship(z_x, z_y, nums);
	nums->color = color_intensity(repeat, nums->max_repeat);
	return (0);
}
