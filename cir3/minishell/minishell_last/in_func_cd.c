/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   in_func_cd.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 16:49:21 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 16:49:23 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	sweet_home(t_env *envs, char **now_path)
{
	*now_path = get_env(envs, "HOME");
	if (!**now_path)
	{
		ft_putstr_fd("cd : HOME not set\n", 2);
		return (set_q(envs, 1), 1);
	}
	return (0);
}

int	in_func_cd(char **args, t_env *envs)
{
	char	*now_path;
	char	*pwd;
	int		argc;

	argc = argv_len(args);
	if (argc > 2)
		return (ft_putstr_fd("cd : too many argvs\n", 2), set_q(envs, 1), 1);
	if (argc == 1)
	{
		if (sweet_home(envs, &now_path))
			return (1);
	}
	else
		now_path = args[1];
	if (chdir(now_path) == 0)
	{
		pwd = get_env(envs, "PWD");
		if (pwd)
			set_env(envs, "OLDPWD", pwd);
		pwd = getcwd(NULL, 0);
		if (!pwd)
			return (perror("cd: malloc fail"), set_q(envs, 1), 1);
		set_env(envs, "PWD", pwd);
		free(pwd);
		return (set_q(envs, 0), 0);
	}
	else
		return (perror("cd"), set_q(envs, 1), 1);
}
