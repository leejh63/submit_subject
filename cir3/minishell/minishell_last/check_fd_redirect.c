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

int	redir_all(t_redir *redirs, int flags)
{
	while (redirs)
	{
		if (redirs->sstr && (!(redirs->sstr)[0] || (redirs->sstr)[1]))
		{
			ft_putstr_fd("bash: ambiguous args\n", 2);
			return (1);
		}
		if (redirs->type == heredoc)
			if (redir_in(redirs->hd_fd))
				return (1);
		if (redirs->type == inrd)
			if (wrap_open(O_RDONLY, (redirs->sstr)[0], redir_in))
				return (1);
		if (redirs->type == outrd)
			if (wrap_open((flags & ~O_APPEND), (redirs->sstr)[0], redir_out))
				return (1);
		if (redirs->type == aprd)
			if (wrap_open((flags & ~O_TRUNC), (redirs->sstr)[0], redir_out))
				return (1);
		redirs = redirs->next;
	}
	return (0);
}

int	check_fd_redirect(t_exp *exp, int in_pipe, int out_pipe)
{
	int	flags;

	flags = O_CREAT | O_WRONLY | O_TRUNC | O_CLOEXEC | O_APPEND;
	if (redir_in(in_pipe))
		return (1);
	if (redir_out(out_pipe))
		return (1);
	if (redir_all(exp->redir, flags))
		return (1);
	if (!exp->args[0])
		return (1);
	return (0);
}
