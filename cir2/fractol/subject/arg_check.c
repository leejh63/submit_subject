/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   arg_check.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/10 21:43:14 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/10 21:56:40 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	type_check(char *argv, char type)
{
	int	len;

	len = 0;
	while (argv[len])
		len++;
	if (len != 1)
		return (1);
	if (argv[0] != type)
		return (1);
	return (0);
}

int	check_window_size(char **argv, int max_width, int max_height)
{
	int	width;
	int	height;

	if (int_check(argv[2]) || int_check(argv[3]))
		return (1);
	if (ft_atoi(argv[2], &width) || ft_atoi(argv[3], &height))
		return (1);
	if (width < 200 || width > max_width || height < 200 || height > max_height)
		return (1);
	return (0);
}

int	check_args(t_mlx *mlx_st, int argc, char **argv)
{
	int	max_width;
	int	max_height;

	if (argc != 4 && argc != 6)
		return (err_num(1));
	mlx_get_screen_size(mlx_st->mlx, &max_width, &max_height);
	if (argc == 4)
	{
		if (mand_arg_check(argv, max_width, max_height))
		{
			mlx_destroy_display(mlx_st->mlx);
			return (free(mlx_st->mlx), free(mlx_st), 2);
		}
	}
	else
	{
		if (julia_arg_check(argv, max_width, max_height))
		{
			mlx_destroy_display(mlx_st->mlx);
			return (free(mlx_st->mlx), free(mlx_st), 2);
		}
	}
	return (0);
}
