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

#include <stdio.h>
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

void test_receive(int signo, siginfo_t *info, void *context)
{
    static unsigned char bit;
    static size_t count;
     static int	check;

    (void)context;

if (!check)
{
	  write(1, "sender ", 7);
    	  ft_putnbr(info->si_pid);
    	  write(1, ": ", 2);
    	  check = 1;
}

    /* 비트 조합 */
    bit <<= 1;
    if (signo == SIGUSR2)
        bit |= 1;
    count++;

    /* 8비트가 모이면 한 문자 완성 */
    if (count == 8)
    {
        write(1, &bit, 1);
      if (bit == '\n')
      {
    		check = 0;
    }
        bit = 0;
        count = 0;
        
    }

    kill(info->si_pid, SIGUSR1);
}

int main(void)
{
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGUSR1);
    sigaddset(&sa.sa_mask, SIGUSR2);
    
    sa.sa_sigaction = test_receive;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    printf("PID: %d\n", getpid());
    while (1)
        pause();

    return 0;
}
