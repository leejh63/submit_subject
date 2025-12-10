/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 14:59:36 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/02 15:31:40 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <signal.h>
#include <stdio.h>

void	test_receive(int sig)
{
	static unsigned char	bit;
	static size_t	count;

	bit <<= 1;
	if (sig == SIGUSR2)
		bit |= 1;
	count++;
	if (count == 8)
	{
		write(1, &bit, 1);
		bit = 0;
		count = 0;
	}
	signal(SIGUSR1, test_receive);
	signal(SIGUSR2, test_receive);
}

int	main(int argc, char **argv)
{
	(void) argc;
	int	i;

	i = 0;
	printf("PID: %d\n", getpid());
	signal(SIGUSR1, test_receive);
	signal(SIGUSR2, test_receive);
	while (1)
		pause();
	return (0);
}
