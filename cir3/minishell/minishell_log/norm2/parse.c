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

static void	*free_exp_and_tok(t_exp *exp, t_tok *tok)
{
	free_exp(exp);
	ftok_lstclear(&tok);
	return (NULL);
}

t_exp	*construct(t_tok *tokens, t_env *envs)
{
	t_tok	*cur;
	t_exp	*head;
	t_exp	*node;

	cur = tokens;
	head = calloc(sizeof(t_exp), 1);
	if (head == NULL)
		return (NULL);
	node = head;
	while (cur && cur->type < end)
	{
		if (!construct_parts(&cur, &head, &node, envs))
			return (free_exp_and_tok(head, tokens));
	}
	if (cur && cur->type == end)
		del_tok(cur);
	node->args = unquote_and_expand_param(node->cmd, envs);
	ftok_lstclear(&node->cmd);
	if (node->args == NULL)
		return (free_exp_and_tok(head, tokens));
	return (head);
}
