/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_exit.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/06 17:34:12 by kkeum             #+#    #+#             */
/*   Updated: 2025/08/25 15:44:52 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cub3d.h"

static void	destroy_resources(void *param)
{
	t_param	*p;

	p = (t_param *)param;
	if (p->img)
		mlx_destroy_image(p->mlx, p->img);
	if (p->rsrc.n)
		mlx_destroy_image(p->mlx, p->rsrc.n);
	if (p->rsrc.e)
		mlx_destroy_image(p->mlx, p->rsrc.e);
	if (p->rsrc.w)
		mlx_destroy_image(p->mlx, p->rsrc.w);
	if (p->rsrc.s)
		mlx_destroy_image(p->mlx, p->rsrc.s);
}

void	free_dptr(char **args)
{
	char	**ptr;

	if (args == NULL)
		return ;
	ptr = args;
	while (*ptr)
	{
		free(*ptr);
		ptr++;
	}
	free(args);
}

int	ft_deallocate(void *param)
{
	t_param	*p;

	p = (t_param *)param;
	if (p->win)
		mlx_destroy_window(p->mlx, p->win);
	destroy_resources(p);
	if (p->mlx)
		mlx_destroy_display(p->mlx);
	free(p->mlx);
	p->mlx = NULL;
	free_dptr(p->map.map);
	p->map.map = NULL;
	return (0);
}

void	ft_exit(t_param *p, const char *s, int err)
{
	perror(s);
	ft_deallocate(p);
	exit(err);
}
