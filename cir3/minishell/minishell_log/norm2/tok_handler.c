/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tok_handler.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 12:17:58 by kkeum             #+#    #+#             */
/*   Updated: 2025/07/16 12:17:59 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_tok	*handle_whitespace(char **line)
{
	size_t	i;
	t_tok	*tok;

	i = 0;
	while (is_whitespace((*line)[i]))
		i++;
	tok = calloc(sizeof(t_tok), 1);
	if (tok == NULL)
		return (NULL);
	tok->next = NULL;
	tok->type = ws;
	tok->str = ft_strndup(*line, i);
	if (!tok->str)
	{
		free(tok);
		return (NULL);
	}
	*line += i;
	return (tok);
}

t_tok	*handle_end(void)
{
	t_tok	*tok;

	tok = calloc(sizeof(t_tok), 1);
	if (tok == NULL)
		return (NULL);
	tok->next = NULL;
	tok->type = end;
	tok->str = NULL;
	return (tok);
}

t_tok	*handle_word(char **line, int in_quote)
{
	size_t	i;
	t_tok	*tok;

	i = 0;
	while (is_general_char((*line)[i], in_quote))
		i++;
	tok = calloc(sizeof(t_tok), 1);
	if (tok == NULL)
		return (NULL);
	tok->next = NULL;
	tok->type = word;
	tok->str = ft_strndup(*line, i);
	if (!tok->str)
	{
		free(tok);
		return (NULL);
	}
	*line += i;
	return (tok);
}

t_tok	*handle_pipe(char **line)
{
	t_tok	*tok;

	tok = calloc(sizeof(t_tok), 1);
	if (tok == NULL)
		return (NULL);
	tok->next = NULL;
	tok->type = pip;
	tok->str = ft_strdup("|");
	if (!tok->str)
	{
		free(tok);
		return (NULL);
	}
	*line += 1;
	return (tok);
}

t_tok	*handle_quote(char **line, int *in_quote)
{
	t_tok	*tok;

	tok = calloc(sizeof(t_tok), 1);
	if (tok == NULL)
		return (NULL);
	tok->next = NULL;
	if (**line == '\'')
	{
		*in_quote = !*in_quote;
		tok->type = sq;
		tok->str = ft_strdup("'");
	}
	else
	{
		*in_quote = !*in_quote * 2;
		tok->type = dq;
		tok->str = ft_strdup("\"");
	}
	if (!tok->str)
	{
		free(tok);
		return (NULL);
	}
	*line += 1;
	return (tok);
}
