/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir_exe_builtin.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 17:18:54 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 17:18:57 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	reset_fd(int *save_fd)
{
	int	i;
	int	j;

	i = dup2(save_fd[0], STDIN_FILENO);
	if (i < 0)
		perror("failed to reset stdin");
	close(save_fd[0]);
	j = dup2(save_fd[1], STDOUT_FILENO);
	if (j < 0)
		perror("failed to reset stdout");
	close(save_fd[1]);
	return (i < 0 || j < 0);
}

void	set_save_fd(int save_fd[2])
{
	save_fd[0] = dup(STDIN_FILENO);
	save_fd[1] = dup(STDOUT_FILENO);
}

int	redir_exe_builtin(t_exp *exp, t_env *envs)
{
	int	save_fd[2];

	set_save_fd(save_fd);
	if (check_fd_redirect(exp, STDIN_FILENO, STDOUT_FILENO))
		return (free_exp(exp), set_q(envs, 1), 1);
	if (check_built_in(exp->args[0]) == 0)
	{
		do_built_in(exp, &envs, STDOUT_FILENO);
		if (ft_strcmp((exp->args)[0], "exit") == 0)
		{
			if (in_func_exit(exp, envs, save_fd[0], save_fd[1]))
				return (1);
			exit(0);
		}
		return (reset_fd(save_fd), free_exp(exp), 0);
	}
	return (reset_fd(save_fd), free_exp(exp), 0);
}
