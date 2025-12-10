/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 12:17:44 by kkeum             #+#    #+#             */
/*   Updated: 2025/07/16 12:17:45 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	construct_redir(t_tok **cur, t_exp	*node, t_env *envs)
{
	t_tok	*next;
	t_redir	*redir;
	t_tok	*wstok;

	redir = calloc(sizeof(t_redir), 1);
	if (redir == NULL)
		return (0);
	redir->type = (*cur)->type;
	next = (*cur)->next;
	del_tok(*cur);
	*cur = next;
	ft_redir_lstadd_back(&node->redir, redir);
	get_one_chunk(cur, &redir->str);
	redir->sstr = unquote_and_expand_param(redir->str, envs);
	ftok_lstclear(&redir->str);
	if (redir->sstr == NULL)
		return (0);
	if (node->cmd == NULL || ftok_lstlast(node->cmd)->type != ws)
	{
		wstok = get_wstok();
		if (wstok == NULL)
			return (0);
		ftok_lstadd_back(&node->cmd, wstok);
	}
	return (1);
}

int	construct_heredocs(t_tok **cur, t_exp *node)
{
	t_tok	*next;
	t_redir	*redir;
	t_tok	*wstok;

	redir = calloc(sizeof(t_redir), 1);
	if (redir == NULL)
		return (0);
	redir->type = heredoc;
	redir->hd_fd = (*cur)->hd_fd;
	ft_redir_lstadd_back(&node->redir, redir);
	next = (*cur)->next;
	del_tok(*cur);
	*cur = next;
	if (node->cmd == NULL || ftok_lstlast(node->cmd)->type != ws)
	{
		wstok = get_wstok();
		if (wstok == NULL)
			return (0);
		ftok_lstadd_back(&node->cmd, wstok);
	}
	return (1);
}

int	construct_pipes(t_tok **cur, t_exp **head, t_exp **node, t_env *envs)
{
	t_tok	*next;

	(*node)->args = unquote_and_expand_param((*node)->cmd, envs);
	ftok_lstclear(&(*node)->cmd);
	if ((*node)->args == NULL)
		return (0);
	*node = calloc(sizeof(t_exp), 1);
	if (*node == NULL)
		return (0);
	fexp_lstadd_back(head, *node);
	next = (*cur)->next;
	del_tok(*cur);
	*cur = next;
	return (1);
}

int	construct_ws(t_tok **cur, t_exp *node)
{
	t_tok	*next;

	next = (*cur)->next;
	(*cur)->next = NULL;
	if (node->cmd && ftok_lstlast(node->cmd)->type != ws)
		ftok_lstadd_back(&node->cmd, *cur);
	else
		del_tok(*cur);
	*cur = next;
	return (1);
}

int	construct_parts(t_tok **cur, t_exp **head, t_exp **node, t_env *envs)
{
	t_tok	*chunk;

	if ((*cur)->type == ws)
		construct_ws(cur, *node);
	else if ((*cur)->type == pip && !construct_pipes(cur, head, node, envs))
		return (0);
	else if ((*cur)->type >= inrd && (*cur)->type <= aprd && \
	!construct_redir(cur, *node, envs))
		return (0);
	else if ((*cur)->type == heredoc && !construct_heredocs(cur, *node))
		return (0);
	else if ((*cur)->type == word | (*cur)->type == sq | \
	(*cur)->type == dq | (*cur)->type == env)
	{
		get_one_chunk(cur, &chunk);
		ftok_lstadd_back(&(*node)->cmd, chunk);
	}
	return (1);
}
