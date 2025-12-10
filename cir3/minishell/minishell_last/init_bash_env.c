/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_bash_env.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 17:01:40 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 17:01:43 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_env	*make_env(char *key, char *val)
{
	t_env	*var;

	var = malloc(sizeof(t_env));
	if (var == NULL)
		return (NULL);
	var->key = key;
	var->val = val;
	var->next = NULL;
	return (var);
}

int	set_env(t_env *env, char *key, char *val)
{
	t_env	*cur;

	if (env == 0)
		return (1);
	cur = env;
	while (cur)
	{
		if (ft_strcmp(cur->key, key) == 0)
		{
			if (cur->val)
				free(cur->val);
			cur->val = ft_strdup(val);
			if (!cur->val)
				return (perror("set_env malloc"), 1);
			return (0);
		}
		cur = cur->next;
	}
	return (0);
}

int	find_equal(char *env_str, int	*equal)
{
	int	i;

	i = 0;
	while (env_str[i])
	{
		if (env_str[i] == '=')
		{
			*equal = i;
			return (1);
		}
		i++;
	}
	*equal = 0;
	return (0);
}

char	*take_env_key(int equal, char *envp)
{
	if (equal)
		return (ft_strndup(envp, equal));
	else
		return (ft_strndup(envp, ft_strlen(envp)));
}

char	*get_env(t_env *envs, char *key)
{
	static const char	*nullstr = "";

	while (envs)
	{
		if (ft_strcmp(envs->key, key) == 0)
		{
			if (envs->val == NULL)
				return ((char *)nullstr);
			return (envs->val);
		}
		envs = envs->next;
	}
	return ((char *)nullstr);
}
