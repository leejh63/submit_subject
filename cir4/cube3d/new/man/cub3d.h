/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cub3d.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/06 17:36:39 by kkeum             #+#    #+#             */
/*   Updated: 2025/08/26 11:32:15 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CUB3D_H
# define CUB3D_H

# include "mlx.h"
# include "mlx_int.h"
# include "ft_printf.h"
# include <fcntl.h>
# include <string.h>
# include <errno.h>
# include <stdio.h>
# include <unistd.h>
# include <math.h>

# define WIN_W 1024
# define WIN_H 1024
# define FOV 90
# define IMG_SIZE 64

typedef union u_color
{
	unsigned char	c[4];
	int				i;
}	t_color;

typedef struct s_map
{
	int		w;
	int		h;
	char	**map;
	t_color	f;
	t_color	c;
}	t_map;

typedef struct s_rsrc
{
	void	*n;
	void	*s;
	void	*w;
	void	*e;
}	t_rsrc;

typedef struct s_view
{
	double	theta;
	double	xpos;
	double	ypos;
	double	verti;
	int		pre_mouse_x;
	int		pre_mouse_y;
}	t_v;

typedef struct s_param
{
	void	*mlx;
	void	*win;
	void	*img;
	t_map	map;
	t_rsrc	rsrc;
	t_v		v;
	int		key_sig;
	int		key[9];
}	t_param;

typedef struct s_ray
{
	double	pos_x;
	double	pos_y;
	int		map_x;
	int		map_y;
	double	ddx;
	double	ddy;
	double	sdx;
	double	sdy;
	int		step_x;
	int		step_y;
	int		hit;
	int		side;
	double	dist;
	double	hit_wall;
	void	*texture;
}	t_ray;

enum
{
	B,
	G,
	R
};

enum
{
	W,
	A,
	S,
	D,
	UP,
	DOWN,
	LEFT,
	RIGHT,
	SPACE
};

int		close_win(void *param);
int		mouse_callback(int x, int y, void *param);
int		releasekey_callback(int key, void *param);
int		presskey_callback(int key, void *param);
int		ft_draw_screen(void *param);
void	parse_map(char *file_name, t_param *p);
void	move(t_param *p);
int		is_surrounded_by_wall(t_map *map);
void	validate_map(t_param *p);
void	ft_exit(t_param *p, const char *s, int err);
t_color	build_color(int red, int green, int blue);
void	ft_pixel_put(t_param *p, int x, int y, int color);
void	free_dptr(char **args);
t_color	parse_color(char *str, t_param *p);
void	dda_ray_cast(double dirX, double dirY, t_param *p, int pix);
t_color	export_color(void	*img, int x, int y);
int		validate_args(int argc, char **argv);
void	update_param(t_param *p, int x, int y, char c);
int		is_all_white(char *s);
int		in_range(int x, int max_x, int y, int max_y);
void	adjust_size(t_param *p);
char	*fill_space(char *s, size_t size);
int		zero_surroundings(t_map *map);
int		ft_deallocate(void *param);
void	draw_column(t_ray *r, t_param *p, int tex_x, int pix_x);
void	draw_empty_column(t_param *p, int pix_x);
#endif
