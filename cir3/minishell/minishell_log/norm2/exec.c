/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 17:16:52 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 17:16:54 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	envs_len(t_env *lst)
{
	int	i;

	i = 0;
	while (lst)
	{
		i++;
		lst = lst->next;
	}
	return (i);
}

char	**env_to_array(t_env *lst)
{
	char	**arr;
	char	**cur;
	int		key_len;
	int		val_len;

	arr = calloc(sizeof(char *), envs_len(lst) + 1);
	cur = arr;
	while (lst)
	{
		key_len = ft_strlen(lst->key);
		val_len = ft_strlen(lst->val);
		*cur = calloc(sizeof(char), key_len + val_len + 2);
		ft_memcpy(*cur, lst->key, key_len);
		ft_memcpy(*cur + key_len, "=", 1);
		ft_memcpy(*cur + key_len + 1, lst->val, val_len);
		cur++;
		lst = lst-> next;
	}
	return (arr);
}

int	exec(t_exp *toks, t_env *envs)
{
	char	**envp;

	envp = env_to_array(envs);
	execve(toks->args[0], toks->args, envp);
	perror("execve fail");
	free_double_ptr(envp);
	return (1);
}
