/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lex.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 12:17:42 by kkeum             #+#    #+#             */
/*   Updated: 2025/07/16 12:17:43 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static t_tok	*get_next_token(char **line, int *in_quote)
{
	char	*str;

	str = *line;
	if (*str == '\0')
		return (handle_end());
	if (is_general_char(*str, *in_quote))
		return (handle_word(line, *in_quote));
	if (*str == '|')
		return (handle_pipe(line));
	if (*str == '\'' && (*in_quote == 0 || *in_quote == 1))
		return (handle_quote(line, in_quote));
	if (*str == '"' && (*in_quote == 0 || *in_quote == 2))
		return (handle_quote(line, in_quote));
	if (*str == '<' || *str == '>')
		return (handle_redirect(line));
	if (*str == '$')
		return (handle_dollar(line));
	if (is_whitespace(*str))
		return (handle_whitespace(line));
	return (NULL);
}

t_tok	*lex(char *line)
{
	t_tok	*tokens;
	t_tok	*token;
	int		in_quote;

	tokens = NULL;
	in_quote = 0;
	while (1)
	{
		token = get_next_token(&line, &in_quote);
		if (token == NULL && *line != '\0')
		{
			perror("minishell: malloc failed while lexing.");
			ftok_lstclear(&tokens);
			break ;
		}
		ftok_lstadd_back(&tokens, token);
		if (token->type == end)
			break ;
	}
	return (tokens);
}
