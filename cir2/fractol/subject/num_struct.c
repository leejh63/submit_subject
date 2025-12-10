/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   num_struct.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/09 10:52:18 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/10 22:05:31 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

void	set_minit_num(t_num *nums, char **argv)
{
	nums -> c_xi = 1.75;
	nums -> c_yi = -1.245;
	nums -> max_x = 2;
	nums -> min_x = -2;
	nums -> max_y = 1.5;
	nums -> min_y = -1.5;
	nums -> str_x = 0;
	nums -> str_y = 0;
	ft_atoi(argv[2], &(nums->width));
	ft_atoi(argv[3], &(nums->height));
	nums -> max_repeat = 500;
	nums -> color = 0;
	nums -> is_mand = 1;
	nums -> scale = 1;
}

void	set_jinit_num(t_num *nums, char **argv)
{
	ft_atof(argv[4], &(nums->c_xi));
	ft_atof(argv[5], &(nums->c_yi));
	nums -> max_x = 2.0;
	nums -> min_x = -2.0;
	nums -> max_y = 1.5;
	nums -> min_y = -1.5;
	nums -> str_x = 0;
	nums -> str_y = 0;
	ft_atoi(argv[2], &(nums->width));
	ft_atoi(argv[3], &(nums->height));
	nums -> max_repeat = 500;
	nums -> color = 0;
	nums -> is_mand = 0;
	nums -> scale = 1;
}

int	set_num(t_num **nums, char **argv)
{
	*nums = malloc(sizeof(t_num));
	if (!*nums)
		return (err_num(5));
	if (argv[1][0] == 'm')
		set_minit_num(*nums, argv);
	else
		set_jinit_num(*nums, argv);
	return (0);
}
