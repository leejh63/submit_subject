/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredo.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kkeum <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/16 12:17:40 by kkeum             #+#    #+#             */
/*   Updated: 2025/07/16 12:17:41 by kkeum            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"
#define HEREDOC_WARN_MSG "warning: here-document delimited by end-of-file"

static char	*get_delim(t_tok **token)
{
	char	*str;
	char	*str2;
	t_tok	*chunk;
	t_tok	*next;

	str = NULL;
	chunk = NULL;
	get_one_chunk(token, &chunk);
	while (chunk)
	{
		next = chunk->next;
		if (chunk->type != dq && chunk->type != sq)
		{
			str2 = ft_strjoin(str, chunk->str);
			if (!str2)
				return (NULL);
			free(str);
			str = str2;
		}
		free(chunk->str);
		free(chunk);
		chunk = next;
	}
	return (str);
}

void	close_all_hd_fd(t_tok *token)
{
	while (token)
	{
		if (token->type == heredoc && token->hd_fd > 0)
			close(token->hd_fd);
		token = token->next;
	}
}

static int	do_heredoc(char *limiter)
{
	char	*line;
	int		hdpipe[2];

	signal(SIGINT, heredoc_sigint);
	if (pipe(hdpipe))
		return (ft_perror("pipe failed.", NULL, NULL, 1), -1);
	while (1)
	{
		set_signum(0);
		line = readline("> ");
		if (get_signum() == SIGINT)
			return (close(hdpipe[0]), close(hdpipe[1]), free(line), -130);
		if (line == NULL && (ft_perror(HEREDOC_WARN_MSG, NULL, NULL, 0), 1))
			break ;
		if (ft_strcmp(line, limiter) == 0)
			break ;
		else
		{
			write(hdpipe[1], line, ft_strlen(line));
			//만약 달러표시 나옴? 확장 후 파이프내에 데이터 삽입
			write(hdpipe[1], "\n", 1);
			free(line);
		}
	}
	return (close(hdpipe[1]), hdpipe[0]);
}

static int	call_heredoc(t_tok *head, t_tok *token)
{
	char	*delim;
	int		ret;

	delim = get_delim(&(token->next));
	if (!delim)
	{
		close_all_hd_fd(head);
		ft_perror("malloc failed.", NULL, NULL, 1);
		return (1);
	}
	token->hd_fd = do_heredoc(delim);
	free(delim);
	if (token->hd_fd < 0)
	{
		close_all_hd_fd(head);
		ret = token->hd_fd * -1;
		return (ret);
	}
	return (0);
}

int	handle_heredocs(t_tok *head)
{
	t_tok	*token;
	int		ret;

	ret = 0;
	token = head;
	while (ret == 0)
	{
		if (token == NULL || token->type == end || token->type == error)
			break ;
		if (token->type == heredoc)
			ret = call_heredoc(head, token);
		token = token->next;
	}
	set_signal_handler();
	return (ret);
}
