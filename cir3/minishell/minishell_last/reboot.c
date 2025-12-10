/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   reboot.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 17:47:44 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 17:47:54 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	next_cur(t_exp **cur)
{
	t_exp	*next;

	next = (*cur)->next;
	(*cur)->next = NULL;
	free_exp(*cur);
	*cur = next;
}

int	pipe_and_fork(t_exp *toks, t_env *envs)
{
	t_exp	*cur;
	int		pid;
	int		pipes[3][2];

	pid = 0;
	cur = toks;
	make_pipe(pipes);
	if (cur->next == NULL && cur->args[0] && check_built_in(cur->args[0]) == 0)
		return (redir_exe_builtin(toks, envs));
	while (cur)
	{
		if (pipe_setting(cur, pipes))
			return (1);
		pid = fork();
		if (pid == -1)
			return (fork_error(pipes, cur), 1);
		else if (pid == 0)
			child_doing(cur, envs, pipes);
		if (pipes[0][0] > 2)
			close(pipes[0][0]);
		next_cur(&cur);
	}
	return (wait_pid_end(pid, envs));
}

int	main(int argc, char **argv, char **envp)
{
	char	*line;
	t_env	*envs;
	t_exp	*exp;

	(void) argc;
	(void) argv;
	envs = NULL;
	if (init_bash(envp, &envs))
		return (1);
	while (1)
	{
		exp = NULL;
		line = readline("minishell$");
		if (line == NULL)
			break ;
		if (is_all_ws(line))
			continue ;
		add_history(line);
		scan_and_parse(line, &exp, envs);
		if (exp)
			pipe_and_fork(exp, envs);
	}
	clear_history();
	free_env(envs);
	return (0);
}
