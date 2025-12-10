/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   validate_syntax.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/14 20:41:29 by kkeum             #+#    #+#             */
/*   Updated: 2025/07/14 20:41:30 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static t_tok	*next_nonws(t_tok *tok)
{
	if (tok == NULL)
		return (NULL);
	while (1)
	{
		tok = tok->next;
		if (tok == NULL)
			return (NULL);
		if (tok->type != ws)
			return (tok);
	}
}

t_tok	*next_tok(t_tok *tok, t_tt type)
{
	while (tok)
	{
		if (tok->type == type)
			return (tok);
		else
			tok = tok->next;
	}
	return (NULL);
}

static void	syntax_error(int *success)
{
	ft_putstr_fd("syntax_error\n", 2);
	*success = 0;
}

static void	check_tok(t_tok **cur, int *success)
{
	t_tok	*next;

	next = next_nonws(*cur);
	if (next == NULL)
		return ;
	if ((*cur)->type == pip)
	{
		if (next->type == pip || next->type == end)
			syntax_error(success);
	}
	else if ((*cur)->type >= inrd && (*cur)->type <= heredoc)
	{
		if (next->type == end || (next->type >= pip && next->type <= heredoc))
			syntax_error(success);
	}
	else if ((*cur)->type == sq || (*cur)->type == dq)
	{
		if (!next_tok((*cur)->next, (*cur)->type))
			syntax_error(success);
		else
			*cur = next_tok((*cur)->next, (*cur)->type);
	}
}

int	validate_syntax(t_tok *toks)
{
	t_tok	*cur;
	int		success;

	success = 1;
	cur = toks;
	if (cur->type == ws)
		cur = next_nonws(cur);
	if (cur->type == pip)
	{
		ft_putstr_fd("syntax_error", 2);
		success = 0;
		cur = next_nonws(cur);
		return (1);
	}
	while (cur)
	{
		if (cur == NULL || cur->type == end)
			break ;
		check_tok(&cur, &success);
		cur = cur->next;
		if (success == 0)
			return (1);
	}
	return (0);
}
