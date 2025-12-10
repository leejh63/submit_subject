/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/11 12:01:23 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/11 12:01:24 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	main(int argc, char **argv)
{
	t_ap	ap;
	t_mlx	*mlx_st;
	t_num	*num_st;

	if (make_mlx(&mlx_st))
		return (1);
	if (check_args(mlx_st, argc, argv))
		return (2);
	if (set_args(&mlx_st, &num_st, &ap, argv))
		return (3);
	fractal_check_dot(mlx_st, num_st);
	mlx_put_image_to_window(mlx_st->mlx, mlx_st->win, mlx_st->img, 0, 0);
	event_set(&ap);
	mlx_loop(mlx_st->mlx);
	destroy_mlx(mlx_st);
	return (free(num_st), 0);
}
