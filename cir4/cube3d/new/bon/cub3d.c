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

static int	load_animation_frame(t_param *p)
{
	int	w;
	int	h;

	p->rsrc.idle = mlx_xpm_file_to_image(p->mlx, "textures/idle.xpm", &w, &h);
	p->rsrc.r1 = mlx_xpm_file_to_image(p->mlx, "textures/run1.xpm", &w, &h);
	p->rsrc.r2 = mlx_xpm_file_to_image(p->mlx, "textures/run2.xpm", &w, &h);
	p->rsrc.r3 = mlx_xpm_file_to_image(p->mlx, "textures/run3.xpm", &w, &h);
	p->dr.d_c = mlx_xpm_file_to_image(p->mlx, "textures/d.xpm", &w, &h);
	p->dr.d_o = mlx_xpm_file_to_image(p->mlx, "textures/do2.xpm", &w, &h);
	//
	p->rsrc.one = mlx_xpm_file_to_image(p->mlx, "textures/1.xpm", &w, &h);
	p->rsrc.two = mlx_xpm_file_to_image(p->mlx, "textures/2.xpm", &w, &h);
	p->rsrc.thr = mlx_xpm_file_to_image(p->mlx, "textures/3.xpm", &w, &h);
	p->rsrc.fur = mlx_xpm_file_to_image(p->mlx, "textures/4.xpm", &w, &h);
	//
	return (p->rsrc.idle && p->rsrc.r1 && p->rsrc.r2 && \
	p->rsrc.r3 && p->dr.d_c && p->dr.d_o);
}

static void	init_mlx_vars(t_param *p)
{
	p->mlx = mlx_init();
	if (!p->mlx)
		ft_exit(p, "mlx_init_fail", 1);
	p->win = mlx_new_window(p->mlx, WIN_W, WIN_H, "cub3d");
	if (!p->win)
		ft_exit(p, "mlx_new_window fail", 1);
	p->img = mlx_new_image(p->mlx, WIN_W, WIN_H);
	if (!p->img)
		ft_exit(p, "mlx_new_image fail", 1);
	if (!load_animation_frame(p))
		ft_exit(p, "Fail to load files.", 1);
}

int	main(int argc, char *argv[])
{
	t_param	p;

	if (validate_args(argc, argv))
		return (0);
	ft_memset(&p, 0, sizeof(p));
	init_mlx_vars(&p);
	parse_map(argv[1], &p);
	mlx_hook(p.win, 2, 1L << 0, presskey_callback, &p);
	mlx_hook(p.win, 3, 1L << 1, releasekey_callback, &p);
	mlx_hook(p.win, 6, 1L << 6, mouse_callback, &p);
	mlx_hook(p.win, 17, 0, close_win, &p);
	mlx_loop_hook(p.mlx, ft_draw_screen, &p);
	mlx_loop(p.mlx);
	ft_deallocate(&p);
	return (0);
}
