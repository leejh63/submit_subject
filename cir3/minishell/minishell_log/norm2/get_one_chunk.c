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

t_tok	*prev_tok(t_tok *head, t_tok *this)
{
	t_tok	*cur;

	if (head == NULL || this == NULL || head == this)
		return (NULL);
	cur = head;
	while (cur->next)
	{
		if (cur->next == this)
			return (cur);
		cur = cur->next;
	}
	return (NULL);
}

t_tok	*get_wstok(void)
{
	t_tok	*tok;

	tok = calloc(sizeof(t_tok), 1);
	if (tok == NULL)
		return (NULL);
	tok->hd_fd = -1;
	tok->type = ws;
	return (tok);
}

t_tok	*drop_first_ws(t_tok *cur)
{
	t_tok	*next;

	while (cur->type == ws)
	{
		next = cur->next;
		cur->next = NULL;
		del_tok(cur);
		cur = next;
	}
	return (cur);
}

void	get_one_chunk(t_tok **tok, t_tok **chunk)
{
	t_tok	*head;
	t_tok	*cur;
	t_tok	*prev;

	cur = drop_first_ws(*tok);
	head = cur;
	while (cur)
	{
		if ((cur->type >= pip && cur->type <= heredoc) || cur->type == ws \
		|| cur->type == end)
		{
			if (head == cur)
				*chunk = NULL;
			else
				*chunk = head;
			*tok = cur;
			prev = prev_tok(head, cur);
			if (prev)
				prev->next = NULL;
			return ;
		}
		else if (cur->type == sq || cur->type == dq)
			cur = next_tok(cur->next, cur->type);
		cur = cur->next;
	}
}
