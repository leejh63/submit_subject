/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fractol.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/08 12:47:27 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/08 19:00:02 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FRACTOL_H
# define FRACTOL_H

# include "mlx.h"
# include "X11/X.h"        // 이벤트 번호, 마스크
# include "X11/keysym.h"
# include <stdlib.h>
# include <math.h>

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

int	set_num(t_num **nums, char **argv);
double	convert_x(int pix_x, t_num *nums);
double	convert_y(int pix_y, t_num *nums);
void	put_color(void *data, int off, t_num *nums, int bgr);
int		offset(int x, int y, int line_len, int bpp);
int		color_intensity(int repeat, int max_repeat);
int		ft_atoi(const char *nptr);
double	ft_atof(const char *nptr);
int		diver_check(double *z_x, double *z_y, double c_x, double c_y);
int		check_mandel(double cx, double cy, t_num *nums);
int		check_julia(double z_xi, double z_yi, t_num *nums);
int		set_mlx(t_mlx **mlx_st, int width, int height);
int		init_mlx(t_mlx *mlx_st, int width, int height);
int		destroy_mlx(t_mlx *mlx_st);
int		mandel_fractal(int p_x, int p_y, t_num *nums);
int		julia_fractal(int p_x, int p_y, t_num *nums);

#endif
