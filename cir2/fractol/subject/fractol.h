/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fractol.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/08 12:47:27 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/10 21:59:14 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FRACTOL_H
# define FRACTOL_H

# include "mlx.h"
# include "X11/X.h"
# include "X11/keysym.h"
# include <stdlib.h>
# include <math.h>
# include <unistd.h>
# include <errno.h>
# include <stdio.h>

typedef struct s_mlx
{
	void	*mlx;
	void	*win;
	void	*img;
	void	*data;
	int		bpp;
	int		line_len;
	int		endian;
}	t_mlx;

typedef struct s_num
{
	double	c_xi;
	double	c_yi;
	double	max_x;
	double	min_x;
	double	max_y;
	double	min_y;
	double	str_x;
	double	str_y;
	double	scale;
	int		width;
	int		height;
	int		max_repeat;
	int		color;
	int		is_mand;
}	t_num;

typedef struct s_ap
{
	t_mlx	*mlx;
	t_num	*num;
}	t_ap;

int		err_arg(int error);
int		err_num(int error);
int		make_mlx(t_mlx **mlx_st);
int		set_args(t_mlx **mlx_st, t_num **num_st, t_ap *ap, char **argv);
int		set_num(t_num **nums, char **argv);
double	convert_x(int pix_x, t_num *nums);
double	convert_y(int pix_y, t_num *nums);
int		offset(int x, int y, int line_len, int bpp);
int		color_intensity(int repeat, int max_repeat);
int		ft_strlen(const char *str);
int		ft_isdigit(int str);
int		ft_atoi(const char *nptr, int *num);
int		ft_atof(const char *nptr, double *num);
int		diver_check(double *z_x, double *z_y, double c_x, double c_y);
int		check_mandel(double cx, double cy, t_num *nums);
int		check_julia(double z_xi, double z_yi, t_num *nums);
int		init_mlx(t_mlx *mlx_st, int width, int height);
int		destroy_mlx(t_mlx *mlx_st);
int		mandel_fractal(int p_x, int p_y, t_num *nums);
int		julia_fractal(int p_x, int p_y, t_num *nums);
void	fractal_check_dot(t_mlx *mlx_st, t_num *num_st);
int		dot_pixel(t_mlx *mlx_st, t_num *num_st, int (*func)(int, int, t_num *));
void	event_set(t_ap *ap);
int		end_program(t_ap *ap);
int		mouse_input(int button, int x, int y, t_ap *ap);
void	zoom_in(int x, int y, t_ap *ap);
void	zoom_out(int x, int y, t_ap *ap);
void	move_mouse(int x, int y, t_ap *ap);
int		key_input(int keycode, t_ap *ap);
void	key_move(int keycode, t_ap *ap);
int		err_num(int error);
int		type_check(char *argv, char type);
int		check_window_size(char **argv, int max_width, int max_height);
int		check_args(t_mlx *mlx_st, int argc, char **argv);
int		float_check(const char *str);
int		julia_arg_check(char **argv, int max_width, int max_height);
int		int_check(char *str);
int		mand_arg_check(char **argv, int max_width, int max_height);
#endif
