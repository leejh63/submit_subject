/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tok_handler2.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 12:17:56 by kkeum             #+#    #+#             */
/*   Updated: 2025/07/16 12:17:57 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	set_redir_tok(t_tok *tok, char **line)
{
	if ((*line)[0] == '<' && (*line)[1] == '<')
	{
		tok->type = heredoc;
		tok->str = ft_strdup("<<");
		*line += 2;
	}
	else if ((*line)[0] == '<')
	{
		tok->type = inrd;
		tok->str = ft_strdup("<");
		*line += 1;
	}
	else if ((*line)[0] == '>' && (*line)[1] == '>')
	{
		tok->type = aprd;
		tok->str = ft_strdup(">>");
		*line += 2;
	}
	else
	{
		tok->type = outrd;
		tok->str = ft_strdup(">");
		*line += 1;
	}
}

t_tok	*handle_redirect(char **line)
{
	t_tok	*tok;

	tok = calloc(sizeof(t_tok), 1);
	if (tok == NULL)
		return (NULL);
	tok->next = NULL;
	set_redir_tok(tok, line);
	if (!tok->str)
	{
		free(tok);
		return (NULL);
	}
	return (tok);
}

static void	set_dollar_tok(t_tok *tok, char **line)
{
	int	i;

	if ((*line)[1] == '?')
	{
		tok->type = env;
		tok->str = ft_strdup("$?");
		*line += 2;
	}
	else if (ft_isalpha((*line)[1]) || (*line)[1] == '_')
	{
		i = 1;
		while (ft_isalnum((*line)[i]) || (*line)[i] == '_')
			i++;
		tok->type = env;
		tok->str = ft_strndup(*line, i);
		*line += i;
	}
	else
	{
		tok->type = word;
		tok->str = ft_strdup("$");
		*line += 1;
	}
}

t_tok	*handle_dollar(char **line)
{
	t_tok	*tok;
	char	*str;

	str = *line;
	tok = calloc(sizeof(t_tok), 1);
	if (tok == NULL)
		return (NULL);
	tok->next = NULL;
	set_dollar_tok(tok, line);
	if (!tok->str)
	{
		free(tok);
		return (NULL);
	}
	return (tok);
}
