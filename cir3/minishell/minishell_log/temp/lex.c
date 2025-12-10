#include "minishell.h"

t_tok	*get_next_token(char **line, int *in_quote)
{
	size_t	i;
	char *str;
	t_tok	*tok;

	str = *line;
	i = 0;
	tok = calloc(sizeof(t_tok), 1);
	if (tok == NULL)
		return (NULL);
	tok->next = NULL;
	if (*str == '\0')
	{
		tok->type = end;
		tok->str = NULL;
	}
	else if (is_general_char(*str, *in_quote))
	{
		while (is_general_char(str[i], *in_quote))
			i++;
		tok->type = word;
		tok->str = ft_strndup(str, i);
		if (!tok->str)
		{
			free(tok);
			return (NULL);
		}
		*line = *line + i;
	}
	else if (*str == '|')
	{
		tok->type = pip;
		tok->str = ft_strdup("|");
		if (!tok->str)
		{
			free(tok);
			return (NULL);
		}
		*line = *line + 1;
	}
	else if (*str == '\'' && (*in_quote == 0 || *in_quote == 1))
	{
		if (*in_quote == 1)
			*in_quote = 0;
		else
			*in_quote = 1;
		tok->type = sq;
		tok->str = ft_strdup("'");
		if (!tok->str)
		{
			free(tok);
			return (NULL);
		}
		*line = *line + 1;
	}
	else if (*str == '"' && (*in_quote == 0 || *in_quote == 2))
	{
		if (*in_quote == 2)
			*in_quote = 0;
		else
			*in_quote = 2;
		tok->type = dq;
		tok->str = ft_strdup("\"");
		if (!tok->str)
		{
			free(tok);
			return (NULL);
		}
		*line = *line + 1;
	}
	else if (*str == '<' && str[1] == '<')
	{
		tok->type = heredoc;
		tok->str = ft_strdup("<<");
		if (!tok->str)
		{
			free(tok);
			return (NULL);
		}
		*line = *line + 2;
	}
	else if (*str == '<' && str[1] != '<')
	{
		tok->type = inrd;
		tok->str = ft_strdup("<");
		if (!tok->str)
		{
			free(tok);
			return (NULL);
		}
		*line = *line + 1;
	}
	else if (*str == '>' && str[1] == '>')
	{
		tok->type = aprd;
		tok->str = ft_strdup(">>");
		if (!tok->str)
		{
			free(tok);
			return (NULL);
		}
		*line = *line + 2;
	}
	else if (*str == '>' && str[1] != '>')
	{
		tok->type = outrd;
		tok->str = ft_strdup(">");
		if (!tok->str)
		{
			free(tok);
			return (NULL);
		}
		*line = *line + 1;
	}
	else if (*str == '>' && str[1] != '>')
	{
		tok->type = outrd;
		tok->str = ft_strdup(">");
		if (!tok->str)
		{
			free(tok);
			return (NULL);
		}
		*line = *line + 1;
	}
	else if (*str == '$')
	{
		if (*str == '?')
		{
			tok->type = env;
			tok->str = ft_strdup("$?");
			if (!tok->str)
			{
				free(tok);
				return (NULL);
			}
			*line = *line + 2;
		}
		else if (ft_isalpha(str[1]) || str[1] == '_')
		{
			i = 1;
			while (ft_isalnum(str[i]) || str[i] == '_')
				i++;
			tok->type = env;
			tok->str = ft_strndup(str, i);
			if (!tok->str)
			{
				free(tok);
				return (NULL);
			}
			*line = *line + i + 1;
		}
		else
		{
			tok->type = word;
			tok->str = ft_strdup("$");
			if (!tok->str)
			{
				free(tok);
				return (NULL);
			}
			*line = *line + 1;
		}
	}
	else if (is_whitespace(*str))
	{
		while (is_whitespace(str[i]))
			i++;
		tok->type = ws;
		tok->str = ft_strndup(str, i);
		if (!tok->str)
		{
			free(tok);
			return (NULL);
		}
		*line = *line + i;
	}
	return (tok);
}

t_tok	*lex(char *line)
{
	t_tok	*tokens;
	t_tok	*token;
	int		in_quote;

	tokens = NULL;
	in_quote = 0;
	while(1)
	{
		token = get_next_token(&line, &in_quote);
		if (token == NULL && *line != '\0')
		{
			ft_perror("malloc fail while lexing.", NULL, NULL, 1);
			ftok_lstclear(&tokens);
			break ;
		}
		ftok_lstadd_back(&tokens, token);
		if (token->type == end)
			break ;
	}
	return (tokens);
}

t_tok	*next_nonws(t_tok *tok)
{
	if (tok == NULL)
		return (NULL);
	while(1)
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
	while(tok)
	{
		if (tok->type == type)
			return (tok);
		else
			tok = tok->next;
	}
	return (NULL);
}

int validate_syntax(t_tok *toks)
{
	t_tok *cur;
	t_tok *next;
	int success = 1;

	cur = toks;
	if (cur->type == ws)
		cur = next_nonws(cur);
	if (cur->type == pip)
	{
		ft_putstr_fd("syntax_error", 2);
		success = 0;
		cur = next_nonws(cur);
	}
	while(cur)
	{
		if (cur->type == end)
			break ;
		next = next_nonws(cur);
		if (next == NULL)
			break ;
		if (cur->type == pip)
		{
			if (next->type == pip || next->type == end)
			{
				ft_putstr_fd("syntax_error:207\n", 2);
				success = 0;
			}
		}
		else if (cur->type >= inrd && cur->type <=heredoc)
		{
			if (next->type == end || (next->type >= pip && next->type <=heredoc))
				ft_putstr_fd("syntax_error:214\n", 2), success = 0;;
		}
		else if (cur->type == sq)
		{
			if (!next_tok(cur->next, sq))
				ft_putstr_fd("syntax_error:219\n", 2), success = 0;
			else
				cur = next_tok(cur->next, sq);
		}
		else if (cur->type == dq)
		{
			if (!next_tok(cur->next, dq))
				ft_putstr_fd("syntax_error:224\n", 2), success = 0;
			else
				cur = next_tok(cur->next, dq);
		}
		if (cur == NULL)
			break ;
		cur = cur->next;
	}
	return (!success);
}
