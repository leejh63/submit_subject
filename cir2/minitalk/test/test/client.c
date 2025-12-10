/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <Jaeholee@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/06 15:33:48 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/06 17:14:31 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

static volatile sig_atomic_t	g_check;

int	ft_isdigit(int str)
{
	unsigned char	unstr;

	if (str < 0 || str > 255)
		return (0);
	unstr = (unsigned char)str;
	return (unstr >= '0' && unstr <= '9');
}

int	ft_atoi(const char *nptr)
{
	int	num;
	int	i;

	if (!nptr)
		return (0);
	i = 0;
	while (nptr[i])
	{
		if (!ft_isdigit(nptr[i]))
			return (-1);
		i++;
	}
	num = 0;
	i = 0;
	while (ft_isdigit(nptr[i]))
	{
		num = num * 10 + (nptr[i] - '0');
		i++;
	}
	return (num);
}

void	send_char(unsigned char message, pid_t pid)
{
	int	i;

	i = 8;
	while (i--)
	{
		if (message & (1 << i))
		{
			write(1, "1", 1);		
			kill(pid, SIGUSR2);
		}
		else
		{
			write(1, "0", 1);
			kill(pid, SIGUSR1);
		}
		while (!g_check)
			pause();
		g_check = 0;
	}
	write(1, " ", 1);
}

void	receive_signal(int signo, siginfo_t *info, void *context)
{
	(void) signo;
	(void) info;
	(void) context;
	g_check = 1;
}

int	main(int argc, char **argv)
{
	struct sigaction	signal;
	int					len;
	int					pid;
	int					i;

	if (argc != 3)
		return (write(1, "Wrong args\n", 11), 1);
	sigemptyset(&signal.sa_mask);
	signal.sa_flags = SA_SIGINFO | SA_RESTART;
	signal.sa_sigaction = receive_signal;
	sigaddset(&signal.sa_mask, SIGQUIT);
	sigaddset(&signal.sa_mask, SIGINT);
	sigaddset(&signal.sa_mask, SIGIO);
	sigaction(SIGUSR1, &signal, NULL);
	pid = ft_atoi(argv[1]);
	if (pid <= 0 || kill(pid, 0) == -1)
		return (write(1, "Wrong pid\n", 10), 1);
	len = 0;
	while (argv[2][len])
		len++;
	i = -1;
	while (++i < len)
		send_char((unsigned char)argv[2][i], pid);
	send_char('\n', pid);
	write(1, "\n", 1);
	return (0);
}
