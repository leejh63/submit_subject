/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   find_path.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/11 16:29:49 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/11 16:29:50 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	free_double_ptr(char **ptr)
{
	char	**dptr;

	if (ptr == NULL)
		return ;
	dptr = ptr;
	while (*dptr)
	{
		free(*dptr);
		dptr++;
	}
	free(ptr);
}

void	free_redir(t_redir *redir)
{
	t_redir	*next;

	while (redir)
	{
		next = redir->next;
		if (redir->type == heredoc && redir->hd_fd >= 0)
			close(redir->hd_fd);
		free_double_ptr(redir->sstr);
		free(redir);
		redir = next;
	}
}

void	free_env(t_env *env)
{
	t_env	*head;

	while (env)
	{
		head = env;
		env = env->next;
		if (head->key)
			free(head->key);
		if (head->val)
			free(head->val);
		free(head);
	}
}

void	free_exp(t_exp *exp)
{
	t_exp	*next;

	while (exp)
	{
		next = exp->next;
		free_double_ptr(exp->args);
		free_redir(exp->redir);
		free(exp);
		exp = next;
	}
}

void	free_exp_envs(t_exp *exp, t_env *envs)
{
	t_exp	*next;

	while (exp)
	{
		next = exp->next;
		free_double_ptr(exp->args);
		free_redir(exp->redir);
		free(exp);
		exp = next;
	}
	free_env(envs);
}
