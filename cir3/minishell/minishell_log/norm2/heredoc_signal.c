/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signal.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 17:49:16 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 17:49:26 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static volatile sig_atomic_t	g_signum = 0;

void	heredoc_sigint(int sig_no)
{
	g_signum = sig_no;
	rl_done = 1;
	write(1, "\n", 1);
}

int	get_signum(void)
{
	return (g_signum);
}

void	set_signum(int sig_no)
{
	g_signum = sig_no;
}

int	null_func(void)
{
	return (0);
}
