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

void	expand_raw_env(t_tok *toks, t_env *envs, t_word **words, t_word **cur)
{
	char	*str;
	int		i;
	char	c;

	str = get_env(envs, toks->str + 1);
	while (*str)
	{
		if (is_whitespace(*str))
		{
			while (is_whitespace(*str) && *str)
				str++;
			word_next(words, cur);
		}
		else
		{
			i = 0;
			while (!is_whitespace(str[i]) && str[i])
				i++;
			c = str[i];
			str[i] = '\0';
			word_append(*cur, str);
			str[i] = c;
			str = str + i;
		}
	}
}

void	unquote_quote(t_tok **toks, t_env *envs, t_word **cur)
{
	if ((*toks)->type == sq)
	{
		*toks = (*toks)->next;
		while ((*toks)->type != sq)
		{
			word_append(*cur, (*toks)->str);
			*toks = (*toks)->next;
		}
	}
	else if ((*toks)->type == dq)
	{
		*toks = (*toks)->next;
		while ((*toks)->type != dq)
		{
			if ((*toks)->type == env)
				word_append(*cur, get_env(envs, (*toks)->str + 1));
			else
				word_append(*cur, (*toks)->str);
			*toks = (*toks)->next;
		}
	}
}

char	**unquote_and_expand_param(t_tok *toks, t_env *envs)
{
	t_word	*words;
	t_word	*cur;
	char	**ret;

	words = NULL;
	cur = NULL;
	word_next(&words, &cur);
	while (toks)
	{
		if (toks->type == sq || toks->type == dq)
			unquote_quote(&toks, envs, &cur);
		else if (toks->type == env)
			expand_raw_env(toks, envs, &words, &cur);
		else if (toks->type == ws)
			word_next(&words, &cur);
		else
			word_append(cur, toks->str);
		toks = toks->next;
	}
	if (cur->str)
		word_next(&words, &cur);
	ret = word_to_str(words);
	return (ret);
}
