/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   in_func_exit.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 16:50:49 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 16:50:49 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	is_numeric(const char *nptr)
{
	while (*nptr == ' ' || *nptr == '\t' || *nptr == '\n' || *nptr == '\r')
		nptr++;
	if (*nptr == '-' || *nptr == '+')
		nptr++;
	if (*nptr < '0' || *nptr > '9')
		return (0);
	while (*nptr >= '0' && *nptr <= '9')
		nptr++;
	while (*nptr == ' ' || *nptr == '\t' || *nptr == '\n' || *nptr == '\r')
		nptr++;
	if (*nptr == '\0')
		return (1);
	else
		return (0);
}

int	in_range(const char *nptr)
{
	char	*limit[2];
	int		len;
	int		sign;

	limit[0] = "9223372036854775807";
	limit[1] = "9223372036854775808";
	while (*nptr == ' ' || *nptr == '\t' || *nptr == '\n' || *nptr == '\r')
		nptr++;
	sign = 0;
	if (*nptr == '-')
		sign = 1;
	if (*nptr == '-' || *nptr == '+')
		nptr++;
	len = 0;
	while (nptr[len] >= '0' && nptr[len] <= '9')
		len++;
	if (len < 20)
		return (1);
	else if (len > 20)
		return (0);
	if (ft_strncmp(nptr, limit[sign], 20) <= 0)
		return (0);
	else
		return (1);
}

int	tmp_atoi(const char *nptr)
{
	long long	ret;
	int			sign;

	ret = 0;
	sign = 1;
	if (*nptr == '-' || *nptr == '+')
	{
		if (*nptr == '-')
			sign = -1;
		nptr++;
		if (!*nptr)
			return (-1);
	}
	while (*nptr >= '0' && *nptr <= '9')
	{
		ret *= 10;
		ret += *nptr - '0';
		nptr++;
	}
	ret = ret * sign;
	return (ret & 0xff);
}

void	exit_func(int in_fd, int out_fd, int exit_num)
{
	close(in_fd);
	close(out_fd);
	exit(exit_num);
}

int	in_func_exit(t_exp *exp, t_env *envs, int in_fd, int out_fd)
{
	int	argc;
	int	exit_num;

	argc = argv_len(exp->args);
	if (argc > 2)
	{
		ft_putstr_fd("exit: Too many args!\n", 2);
		set_q(envs, 1);
		return (free_exp(exp), close(in_fd), close(out_fd), 1);
	}
	if (argc == 1)
	{
		free_exp_envs(exp, envs);
		exit_func(in_fd, out_fd, 0);
	}
	if (!is_numeric(exp->args[1]) || !in_range(exp->args[1]))
	{
		ft_putstr_fd("exit: numeric argument required\n", 2);
		set_q(envs, 2);
		return (free_exp(exp), close(in_fd), close(out_fd), 1);
	}
	else
		exit_num = tmp_atoi(exp->args[1]);
	free_exp_envs(exp, envs);
	return (exit_func(in_fd, out_fd, exit_num), 0);
}
