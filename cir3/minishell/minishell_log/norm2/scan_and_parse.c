/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scan_and_parse.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 17:13:18 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 17:13:20 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	is_all_ws(char *str)
{
	char	*ptr;

	ptr = str;
	if (str == NULL)
		return (-1);
	while (*str)
	{
		if (is_whitespace(*str))
			str++;
		else
			return (0);
	}
	return (free(ptr), 1);
}

int	too_many_heredoc(t_tok	*toks)
{
	int	cnt;

	cnt = 0;
	while (toks)
	{
		if (toks->type == heredoc)
			cnt++;
		toks = toks->next;
	}
	if (cnt > 16)
	{
		ft_perror("maximum here-document count exceeded", NULL, NULL, 0);
		return (2);
	}
	else
		return (0);
}

void	hd_free_and_exit(t_tok *toks, t_env *envs, int status)
{
	ftok_lstclear(&toks);
	clear_history();
	free_env(envs);
	exit(status);
}

int	parse_status(t_env *envs, t_tok *toks)
{
	int	status;

	status = handle_heredocs(toks);
	if (status)
	{
		set_q(envs, status);
		ftok_lstclear(&toks);
		return (1);
	}
	return (0);
}

void	scan_and_parse(char *line, t_exp **chunks, t_env *envs)
{
	t_tok	*toks;

	toks = NULL;
	*chunks = NULL;
	toks = lex(line);
	free(line);
	if (toks == NULL)
		return ;
	if (validate_syntax(toks))
	{
		ftok_lstclear(&toks);
		set_q(envs, 2);
		return ;
	}
	if (too_many_heredoc(toks))
		hd_free_and_exit(toks, envs, 2);
	if (parse_status(envs, toks))
		return ;
	*chunks = construct(toks, envs);
	if (chunks == NULL)
	{
		ft_perror("malloc failed while parsing.", NULL, NULL, 1);
		set_q(envs, 1);
	}
}
