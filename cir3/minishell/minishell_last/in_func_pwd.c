/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   in_func_pwd.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 17:06:46 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 17:06:47 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	in_func_pwd(char **argv, t_env *envs, int out_fd)
{
	int		argc;
	char	*pwd;

	argc = argv_len(argv);
	if (argc != 1)
	{
		ft_putstr_fd("pwd wrong args!\n", 2);
		return (set_q(envs, 1), 1);
	}
	pwd = getcwd(NULL, 0);
	ft_putendl_fd(pwd, out_fd);
	free(pwd);
	return (set_q(envs, 0), 0);
}
