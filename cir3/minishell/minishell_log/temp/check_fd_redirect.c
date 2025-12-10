/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_fd_redirect.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/13 15:45:00 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/13 15:45:01 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	redir_in(int redir_fd)
{
	ft_putnbr_fd(redir_fd * -1, 2);
	if (redir_fd != STDIN_FILENO)
	{
		if (dup2(redir_fd, STDIN_FILENO) < 0)
			return (perror("change_in, dup2"), close(redir_fd), 1);
		close(redir_fd);
	}
	return (0);
}

int	redir_out(int redir_fd)
{
	ft_putnbr_fd(redir_fd, 2);
	if (redir_fd != STDOUT_FILENO)
	{
		if (dup2(redir_fd, STDOUT_FILENO) < 0)
			return (perror("change_out, dup2"), close(redir_fd), 1);
		close(redir_fd);
	}
	return (0);
}

int	wrap_open(int flags, char *path, int (*f_redir)(int))
{
	int	fd;

	fd = open(path, flags, 0644);
	if (fd < 0)
		return (perror("open"), 1);
	if (f_redir(fd))
		return (1);
	return (0);
}

int	redir_all(t_redir *redirs)
{
	int	out_flags;

	out_flags = O_CREAT | O_WRONLY | O_TRUNC | O_CLOEXEC;

	while (redirs)
	{
		if (redirs->type == heredoc)
		{
			if (redir_in(redirs->hd_fd))
				return (1);
		}
		else if ((redirs->sstr)[0] == NULL)
		{
			printf("ambiguous args\n");
			return (1);
		}
		else if ((redirs->sstr)[1] != NULL)
		{
			printf("ambiguous args\n");
			return (1);
		}
		else if (redirs->type == inrd)
		{
			if (wrap_open(O_RDONLY, (redirs->sstr)[0], redir_in))
				return (1);
		}
		else if (redirs->type == outrd)
		{
			if (wrap_open(out_flags, (redirs->sstr)[0], redir_out))
				return (1);		
		}
		else if (redirs->type == aprd)
		{
			if (wrap_open((out_flags & ~O_TRUNC) | O_APPEND, (redirs->sstr)[0], redir_out))
				return (1);
		}
		redirs = redirs->next;
	}
	return (0);
}

int	check_fd_redirect(t_exp *exp, int in_pipe, int out_pipe)
{
	if (in_pipe < 0 || out_pipe < 0)
	{
		printf("check_fd_redirect, wrong in/out_pipe\n");
		return (1);
	}
	// 초기 리다이렉션 진행
	if (redir_in(in_pipe))
		return (1);
	if (redir_out(out_pipe))
		return (1);
	// t_redir 순회 및 리다이렉션	
	if (redir_all(exp->redir))
		return (1);
	return (0);
}
