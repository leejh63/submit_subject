/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_bash.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 16:54:56 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 16:54:57 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	make_first_env(t_env **lst)
{
	t_env	*var;
	char	*key;
	char	*val;

	key = ft_strdup("?");
	if (key == NULL)
		return (1);
	val = ft_calloc(1, 4);
	if (val == NULL)
		return (1);
	val[0] = '0';
	var = make_env(key, val);
	if (var == NULL)
		return (1);
	ft_env_add_back(lst, var);
	return (0);
}

int	set_t_env(char *envp, char **val, char **key)
{
	int	equal;
	int	slen;

	if (find_equal(envp, &equal))
	{
		slen = ft_strlen(&envp[equal + 1]);
		*val = ft_strndup(&envp[equal + 1], slen);
		if (*val == NULL)
			return (1);
	}
	else
		*val = NULL;
	*key = take_env_key(equal, envp);
	if (*key == NULL)
		return (1);
	return (0);
}

int	make_t_env(char **envp, t_env **lst)
{
	t_env	*var;
	char	*key;
	char	*val;
	int		i;

	i = 0;
	if (envp)
	{
		while (envp[i])
		{
			if (set_t_env(envp[i], &val, &key))
				return (1);
			var = make_env(key, val);
			if (!var)
				return (1);
			ft_env_add_back(lst, var);
			i++;
		}
	}
	return (0);
}

int	set_pwd(t_env **lst)
{
	t_env	*var;
	char	*val;
	char	*key;

	val = get_env(*lst, "PWD");
	if (!val)
	{
		val = ft_strdup("PWD");
		if (!val)
			return (perror("set_pwd, ft_strdup"), 1);
		key = getcwd(NULL, 0);
		if (!key)
			return (perror("set_pwd, getcwd"), 1);
		var = make_env(val, key);
		if (!var)
			return (perror("set_pwd, make_env"), free(key), free(val), 1);
		ft_env_add_back(lst, var);
	}
	return (0);
}

int	init_bash(char **envp, t_env **lst)
{
	rl_event_hook = null_func;
	rl_outstream = stderr;
	set_signal_handler();
	if (make_first_env(lst))
		return (1);
	if (make_t_env(envp, lst))
		return (1);
	if (set_pwd(lst))
		return (1);
	if (set_shl_lv(lst))
		return (1);
	return (0);
}
