/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fork_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 17:11:52 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 17:11:54 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	fork_error(int pipes[3][2], t_exp *cur)
{
	if (pipes[0][0] > 2)
		close(pipes[0][0]);
	if (pipes[1][0] > 2)
		close(pipes[1][0]);
	if (pipes[1][1] > 2)
		close(pipes[1][1]);
	free_exp(cur);
	ft_putstr_fd("fork error\n", 2);
}

void	built_exit_fork(t_exp *cur, t_env *envs, int pipes[3][2])
{
	int	exit_num;

	exit_num = do_built_in(cur, &envs, STDOUT_FILENO);
	if (ft_strcmp((cur->args)[0], "exit") == 0)
	{
		close(pipes[0][1]);
		in_func_exit(cur, envs, pipes[0][0], pipes[1][1]);
		exit_num = get_q(envs);
	}
	free_exp_envs(cur, envs);
	exit (exit_num);
}

int	wait_pid_end(int pid, t_env *envs)
{
	int		status;
	int		ret;

	if (waitpid(pid, &status, 0) < 0)
	{
		perror("waitpid");
		ret = 1;
	}
	else if (WIFEXITED(status))
		ret = WEXITSTATUS(status);
	else if (WIFSIGNALED(status))
		ret = 128 + WTERMSIG(status);
	else
		ret = 1;
	while (wait(NULL) > 0)
		;
	set_q(envs, ret);
	return (ret);
}

void	child_doing(t_exp *cur, t_env *envs, int pipes[3][2])
{
	if (pipes[1][0] > 2)
		close (pipes[1][0]);
	if (check_fd_redirect(cur, pipes[0][0], pipes[1][1]))
	{
		free_exp_envs(cur, envs);
		exit (1);
	}
	if (check_built_in(cur->args[0]) == 0)
		built_exit_fork(cur, envs, pipes);
	else
	{
		if (check_find_path(&cur->args[0], envs))
		{
			free_exp_envs(cur, envs);
			exit (127);
		}
		exec(cur, envs);
	}
	free_exp_envs(cur, envs);
	exit (126);
}
