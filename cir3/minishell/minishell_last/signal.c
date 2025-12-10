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

void	sig_int_func(int sig_no)
{
    (void)sig_no;
	g_signum = 1;
}

void	set_signal_handler(void)
{
	signal(SIGQUIT, SIG_IGN);
	if (isatty(1))
		signal(SIGINT, sig_int_func);
	else
		signal(SIGINT, SIG_IGN);
}

int on_rl_event(void)
{
    if (g_signum)
    {
        write(1, "\n", 1);
        rl_replace_line("", 0);
        rl_on_new_line();
        rl_redisplay();
        g_signum = 0;
    }
    return 0;
}
