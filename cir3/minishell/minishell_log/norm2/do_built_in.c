/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   do_built_in.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 17:51:52 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 17:51:59 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	argv_len(char **argv)
{
	int	i;

	i = 0;
	while (argv[i])
		i++;
	return (i);
}

int	check_built_in(char *args)
{
	if (ft_strcmp(args, "env") == 0)
		return (0);
	else if (ft_strcmp(args, "pwd") == 0)
		return (0);
	else if (ft_strcmp(args, "echo") == 0)
		return (0);
	else if (ft_strcmp(args, "unset") == 0)
		return (0);
	else if (ft_strcmp(args, "export") == 0)
		return (0);
	else if (ft_strcmp(args, "exit") == 0)
		return (0);
	else if (ft_strcmp(args, "cd") == 0)
		return (0);
	else
		return (1);
}

int	do_built_in(t_exp *exp, t_env **envs, int out_fd)
{
	if (ft_strcmp((exp->args)[0], "env") == 0)
		return (in_func_env(exp->args, *envs, out_fd));
	else if (ft_strcmp((exp->args)[0], "pwd") == 0)
		return (in_func_pwd(exp->args, *envs, out_fd));
	else if (ft_strcmp((exp->args)[0], "echo") == 0)
		return (in_func_echo(exp->args, *envs, out_fd));
	else if (ft_strcmp((exp->args)[0], "unset") == 0)
		return (in_func_unset(exp->args, envs, out_fd));
	else if (ft_strcmp((exp->args)[0], "export") == 0)
		return (in_func_export(exp->args, envs, out_fd));
	else if (ft_strcmp((exp->args)[0], "cd") == 0)
		return (in_func_cd(exp->args, *envs));
	else
		return (1);
}
