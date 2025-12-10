/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   err_num.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/11 12:02:15 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/11 12:02:16 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	err_arg(int error)
{
	if (error == 1)
	{
		write(2, "possible arg \n", 15);
		write(2, "m width height\n", 16);
		write(2, "j width height real complex\n", 29);
		return (1);
	}
	if (error == 2)
	{
		write(2, "wrong type\n", 11);
		return (2);
	}
	if (error == 3)
	{
		write(2, "wrong width/height\n", 19);
		return (3);
	}
	if (error == 4)
	{
		write(2, "wrong real/complex args\n", 24);
		return (4);
	}
	return (5);
}

int	err_num(int error)
{
	if (error == 0)
	{
		write(2, "init mlx fail\n", 14);
		return (1);
	}
	if (error >= 1 && error <= 4)
	{
		return (err_arg(error));
	}
	if (error == 5)
	{
		perror("malloc fail\n");
		return (5);
	}
	return (-1);
}
