/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   basic.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 09:45:00 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/02 09:45:02 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <readline/readline.h>	// readline 관련 헤더
#include <readline/history.h>	// history 관련 헤더
#include <unistd.h>		// write 관련 헤더
#include <stdio.h>		// printf, perror 관련 헤더
#include <signal.h>		// signal 관련 헤더
#include <errno.h>		// err_num 관련 헤더 
					//	perror의 경우 <errno.h> 헤더 포함이 표준
#include <stdlib.h>		// free, malloc 관련 헤더
#include <sys/ioctl.h>		// ioctl() prototypes, TIOCSTI 정의 관련 헤더


static volatile sig_atomic_t sig_int;

void	ctrl_c(void)
{
	write(1, "\n", 1);
	rl_replace_line("", 0);
	rl_on_new_line();
	rl_redisplay();
	sig_int = 0;
}

void	sig_int_func(int sig_no, siginfo_t *info, void *ctx)
{
	(void) sig_no;
	(void) info;
	(void) ctx;
	
	sig_int = sig_no;
	if (sig_int == SIGINT)
		ctrl_c();
}

int	signal_set(void)
{
	struct sigaction sig;

	sig.sa_sigaction = sig_int_func;
	sig.sa_flags = SA_SIGINFO;
	if (sigemptyset(&sig.sa_mask) == -1)
		return (perror("sig_empty_set"), 1);
	if (sigaddset(&sig.sa_mask, SIGTERM) == -1)
		return (perror("sig_add_set"), 1);
	if (sigaction(SIGINT, &sig, NULL) == -1)
		return (perror("sig_action"), 1);
	return (0);
}

int	main(void)
{
	
	char	*read_line;
	
	if (signal_set())
		return (1);
	while (1)
	{
		read_line = readline("readline>");
		if (!read_line)
		{
			if (errno == 0) //ctrl+D
				return (write(1, "exit\n", 5), 0);
			perror("readline");
			return (1);
		}
		else
		{
			if (*read_line)
			{
				printf("%s\n", read_line);
				add_history(read_line);
			}
		}
		free(read_line);
	}
}
