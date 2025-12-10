/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mandel_fractal.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/08 21:38:06 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/08 21:38:07 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	mandel_fractal(int p_x, int p_y, t_num *nums)
{
	double	z_x;
	double	z_y;
	int		repeat;

	z_x = convert_x(p_x, nums);
	z_y = convert_y(p_y, nums);
	repeat = check_mandel(z_x, z_y, nums);
	nums->color = color_intensity(repeat, nums->max_repeat);
	return (0);
}
