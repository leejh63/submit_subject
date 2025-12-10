/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/06 16:50:37 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/06 16:50:38 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void	ft_putnbr(int n)
{
	int		num;
	int		mod;
	char	numchar;

	num = n / 10;
	mod = n % 10;
	if (num)
		ft_putnbr(num);
	numchar = mod + '0';
	write(1, &numchar, 1);
}

void receive(int signal, siginfo_t *info, void *context)
{
	(void) context;
	static unsigned char	bit;
	static size_t		count;
	static int			check;

	if (!check)
	{
		write(1, "sender ", 7);
		ft_putnbr(info->si_pid);
		write(1, ": ", 2);
		check = 1;
	}
	bit <<= 1;
	if (signal == SIGUSR2)
		bit |= 1;
	count++;
	if (count == 8)
	{
		write(1, &bit, 1);
		if (bit == '\n')
			check = 0;
		count = 0;
		bit = 0;
	}
	kill(info->si_pid, SIGUSR1);
}

int main(void)
{
	struct sigaction sa;

	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGUSR1);
	sigaddset(&sa.sa_mask, SIGUSR2);
	sa.sa_sigaction = receive;
	sa.sa_flags = SA_SIGINFO | SA_RESTART;
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);
	write(1, "SERVER_PID: ", 12);
	ft_putnbr(getpid());
	write(1, "\n", 1);
	while (1)
		pause();
	return 0;
}
