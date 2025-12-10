/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cub3d.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/28 17:13:50 by kkeum             #+#    #+#             */
/*   Updated: 2025/08/25 15:44:48 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

int	main(int argc, char *argv[])
{
	t_param	p;

	if (validate_args(argc, argv))
		return (0);
	ft_memset(&p, 0, sizeof(p));
	p.mlx = mlx_init();
	parse_map(argv[1], &p);
	if (!p.mlx)
		ft_exit(&p, "mlx_init_fail", 1);
	p.win = mlx_new_window(p.mlx, WIN_W, WIN_H, "cub3d");
	if (!p.win)
		ft_exit(&p, "mlx_new_window fail", 1);
	p.img = mlx_new_image(p.mlx, WIN_W, WIN_H);
	if (!p.img)
		ft_exit(&p, "mlx_new_image fail", 1);
	ft_draw_screen(&p);
	mlx_hook(p.win, 2, 1L << 0, presskey_callback, &p);
	mlx_hook(p.win, 3, 1L << 1, releasekey_callback, &p);
	mlx_hook(p.win, 6, 1L << 6, mouse_callback, &p);
	mlx_hook(p.win, 17, 0, close_win, &p);
	mlx_loop_hook(p.mlx, ft_draw_screen, &p);
	mlx_loop(p.mlx);
	ft_deallocate(&p);
	return (0);
}
