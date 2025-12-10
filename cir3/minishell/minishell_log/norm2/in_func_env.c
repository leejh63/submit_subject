/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   in_func_env.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 17:07:12 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 17:07:14 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	find_envlist_key(t_env *envs, char *key, char *val)
{
	while (envs)
	{
		if (ft_strcmp(envs->key, key) == 0)
		{
			free(envs->val);
			envs->val = val;
			return (1);
		}
		envs = envs->next;
	}
	return (0);
}

int	check_env_name(char *key, int *ex_sig)
{
	int	i;

	if (!ft_isalpha(key[0]) && key[0] != '_')
	{
		*ex_sig = 1;
		return (1);
	}
	i = 1;
	while (key[i] && key[i] != '=')
	{
		if (!ft_isalnum(key[i]) && key[i] != '_')
		{
			*ex_sig = 1;
			return (1);
		}
		i++;
	}
	return (0);
}

int	in_func_env(char **argv, t_env *envsf, int out_fd)
{
	int		argc;
	t_env	*envs;

	envs = envsf->next;
	argc = argv_len(argv);
	if (argc != 1)
	{
		ft_putstr_fd("env: wrong args!\n", 2);
		return (set_q(envsf, 1), 1);
	}
	while (envs)
	{
		if (envs->val)
		{
			write(out_fd, envs->key, ft_strlen(envs->key));
			write(out_fd, "=", 1);
			write(out_fd, envs->val, ft_strlen(envs->val));
			write(out_fd, "\n", 1);
		}
		envs = envs->next;
	}
	return (set_q(envsf, 0), 0);
}
