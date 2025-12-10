/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   in_func_echo.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 17:06:06 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 17:06:07 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	echo_write(char **argv, int out_fd, int argc)
{
	int	i;

	i = 0;
	while (argv[i])
	{
		write(out_fd, argv[i], ft_strlen(argv[i]));
		if (argc != i)
			write(out_fd, " ", 1);
		i++;
	}
}

int	in_func_echo(char **argv, t_env *envs, int out_fd)
{
	int	argc;

	argc = argv_len(argv) - 1;
	if (argc == 0)
		return (write(out_fd, "\n", 1), set_q(envs, 0), 0);
	if (ft_strcmp(argv[1], "-n") == 0)
		echo_write(&argv[2], out_fd, argc - 1);
	else
	{
		echo_write(&argv[1], out_fd, argc - 1);
		write(out_fd, "\n", 1);
	}
	return (set_q(envs, 0), 0);
}
