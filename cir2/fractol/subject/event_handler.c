/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   event_handler.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/10 11:19:06 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/10 21:57:58 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int	end_program(t_ap *ap)
{
	destroy_mlx(ap->mlx);
	free(ap->num);
	exit (0);
}

void	event_set(t_ap *ap)
{
	mlx_hook((*ap).mlx->win, 2, 1L << 0, key_input, (void *)ap);
	mlx_hook((*ap).mlx->win, 4, 1L << 2, mouse_input, (void *)ap);
	mlx_hook((*ap).mlx->win, 17, 1L << 17, end_program, (void *)ap);
}
