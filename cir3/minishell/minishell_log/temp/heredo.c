#include "minishell.h"

char *get_delim(t_tok **token)
{
	char *str;
	char *str2;
	t_tok *chunk;
	t_tok *next;

	str = NULL;
	chunk = NULL;
	get_one_chunk(token, &chunk);
	while(chunk)
	{
		next = chunk->next;
		if (chunk->type != dq && chunk->type != sq)
		{
			str2 = ft_strjoin(str, chunk->str);
			if (!str2)
				return (NULL) ;
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

int	do_heredoc(char *limiter) //기존의 hd pipe 닫기?? file open할 때 마지막에 닫자
{
	char *line, *prompt;

	signal(SIGINT, set_signum);
	prompt = ">";
	int hdpipe[2];
	if (pipe(hdpipe))
		return (ft_perror("pipe failed.", NULL, NULL, 1), -1);
	while(1)
	{
		line = readline(prompt);
		if (g_signum == SIGINT)
		{
			//int status = 128 + SIGINT;
			close(hdpipe[0]);
			close(hdpipe[1]);
			free(line);
			return (-130);
		}
		if (line == NULL)
		{
			ft_perror("warning: here-document delimited by end-of-file (wanted `", limiter, "')", 0);
			break ;
		}
		if (ft_strcmp(line, limiter) == 0)
			break ;
		else
		{
			write(hdpipe[1], line, ft_strlen(line));
			write(hdpipe[1], "\n", 1);
			free(line);
		}
	}
	close(hdpipe[1]);
    return (hdpipe[0]);
}

int	handle_heredocs(t_tok *head)
{
	char	*delim;
	int		ret;
	t_tok	*token;

	token = head;
	ret = 0;
	while (1) //아마 링크드 리스트로 하지 않을까
	{
		if (token == NULL || token->type == end || token->type == error)
			break ;
		if (token->type == heredoc)
		{
			delim = get_delim(&(token->next));
			if (!delim)
			{
				close_all_hd_fd(head);
				ft_perror("malloc failed.", NULL, NULL, 1);
				ret = 1;
				break ;
			}
			token->hd_fd = do_heredoc(delim);
			free(delim);
			if (token->hd_fd < 0)
			{
				close_all_hd_fd(head);
				ret = token->hd_fd * -1;
				break ;
			}
		}
		token = token->next;
	}
	set_signal_handler();
	return (ret);
}
