#include "minishell.h"

void free_double_ptr(char **ptr)
{
	char **dptr;

	if (ptr == NULL)
		return ;
	dptr = ptr;
	while(*dptr)
	{
		free(*dptr);
		dptr++;
	}
	free(ptr);
}

void	free_redir(t_redir *redir)
{
	t_redir	*next;

	while(redir)
	{
		next = redir->next;
		if (redir->type == heredoc && redir->hd_fd >= 0)
			close(redir->hd_fd);
		free_double_ptr(redir->sstr);
		free(redir);
		redir = next;
	}
}

void	free_exp(t_exp *exp)
{
	t_exp	*next;

	while(exp)
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

	while(exp)
	{
		next = exp->next;
		free_double_ptr(exp->args);
		free_redir(exp->redir);
		free(exp);
		exp = next;
	}
	free_env(envs);
}













