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

static void	destroy_resources(t_param *p)
{
	int			i;
	const void	*ptr[15] = {p->img, p->rsrc.n, p->rsrc.e, p->rsrc.w, p->rsrc.s, \
		p->rsrc.idle, p->rsrc.r1, p->rsrc.r2, p->rsrc.r3, p->dr.d_o, p->dr.d_c, \
		p->rsrc.one, p->rsrc.two, p->rsrc.thr, p->rsrc.fur};

	i = 0;
	while (i < 15)
	{
		if (ptr[i])
			mlx_destroy_image(p->mlx, (void *)ptr[i]);
		i++;
	}
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
