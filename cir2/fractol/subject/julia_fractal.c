/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   julia_fractal.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/08 21:55:21 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/10 22:01:34 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	julia_fractal(int p_x, int p_y, t_num *nums)
{
	double	z_x;
	double	z_y;
	int		repeat;

	z_x = convert_x(p_x, nums);
	z_y = convert_y(p_y, nums);
	repeat = check_julia(z_x, z_y, nums);
	nums->color = color_intensity(repeat, nums->max_repeat);
	return (0);
}
