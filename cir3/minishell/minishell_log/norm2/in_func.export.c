/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   in_func.export.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 16:52:20 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 16:52:21 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	print_one_env(t_env *envs, int out_fd)
{
	write(out_fd, "declare -x ", 11);
	write(out_fd, envs->key, ft_strlen(envs->key));
	if (envs->val)
	{
		write(out_fd, "=", 1);
		write(out_fd, envs->val, ft_strlen(envs->val));
	}
	write(out_fd, "\n", 1);
}

void	write_export(t_env *envs, int out_fd)
{
	char	*last_key;
	t_env	*start;
	t_env	*p_one;
	t_env	*j;

	last_key = "";
	start = envs;
	while (start)
	{
		j = envs;
		p_one = NULL;
		while (j)
		{
			if (ft_strcmp(last_key, j->key) < 0)
				if (!p_one || ft_strcmp(p_one->key, j->key) > 0)
					p_one = j;
			j = j->next;
		}
		print_one_env(p_one, out_fd);
		last_key = p_one->key;
		start = start->next;
	}
}

int	export_env_val(char *argv, char **val, int *equal)
{
	int	slen;

	if (find_equal(argv, equal))
	{
		slen = ft_strlen(&argv[*equal + 1]);
		*val = ft_strndup(&argv[*equal + 1], slen);
		if (val == NULL)
			return (perror("export malloc"), -1);
	}
	else
		*val = NULL;
	return (1);
}

int	env_key_var(char *val, int equal, char *argv, t_env **envs)
{
	t_env	*var;
	char	*key;

	key = take_env_key(equal, argv);
	if (!key)
		return (1);
	if (find_envlist_key(*envs, key, val))
		return (free(key), 0);
	var = make_env(key, val);
	if (!var)
		return (1);
	ft_env_add_back(envs, var);
	return (0);
}

int	in_func_export(char **argv, t_env **envsf, int out_fd)
{
	int		i;
	int		signal;
	int		equal;
	char	*val;
	t_env	**envs;

	envs = &(*envsf)->next;
	if (argv[1] == NULL)
		return (write_export(*envs, out_fd), set_q((*envsf), 0), 0);
	i = 0;
	signal = 0;
	while (argv[++i])
	{
		if (check_env_name(argv[i], &signal))
			continue ;
		if (export_env_val(argv[i], &val, &equal) < 0)
			return (set_q((*envsf), 1), 1);
		if (env_key_var(val, equal, argv[i], envs))
			return (set_q((*envsf), 1), 1);
	}
	if (signal)
		ft_putstr_fd("export: invalid identifier\n", 2);
	return (set_q(*envsf, signal), signal);
}
