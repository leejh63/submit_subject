/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipe_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 17:10:44 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 17:10:45 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	make_pipe(int pipes[3][2])
{
	int	i;

	i = 0;
	while (i < 3)
	{
		pipes[i][0] = 0;
		pipes[i][1] = 1;
		i++;
	}
}

int	pipe_setting(t_exp *cur, int pipes[3][2])
{
	ft_memcpy(pipes[0], pipes[1], sizeof(int) * 2);
	ft_memcpy(pipes[1], pipes[2], sizeof(int) * 2);
	if (pipes[0][1] > 2)
		close(pipes[0][1]);
	if (cur->next)
		if (pipe(pipes[1]))
			return (pipe_error(pipes, cur), 1);
	return (0);
}

void	pipe_error(int pipes[3][2], t_exp *cur)
{
	if (pipes[0][0] > 2)
		close(pipes[0][0]);
	free_exp(cur);
	ft_putstr_fd("\n\npipe erro\n\n", 2);
}
