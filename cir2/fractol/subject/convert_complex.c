/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   convert_complex.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/08 11:52:28 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/08 11:52:31 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

double	convert_x(int pix_x, t_num *nums)
{
	double	c_x;
	double	scale_x;

	scale_x = (nums->max_x - nums->min_x) / (nums->width);
	c_x = nums->min_x + (pix_x + 0.5) * scale_x + nums->str_x;
	return (c_x);
}

double	convert_y(int pix_y, t_num *nums)
{
	double	c_y;
	double	scale_y;

	scale_y = (nums->max_y - nums->min_y) / (nums->height);
	c_y = nums->max_y - (pix_y + 0.5) * scale_y + nums->str_y;
	return (c_y);
}
