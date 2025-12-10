/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cla.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 16:35:39 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/06/02 16:35:41 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t g_check;

void send_char(unsigned char message, pid_t pid)
{
    int i = 8;

    while (i--)
    {
        if (message & (1 << i))
            kill(pid, SIGUSR2);
        else
            kill(pid, SIGUSR1);
        while (!g_ack)
            pause();
        g_ack = 0;
    }
}

void receive_sig1(int signo, siginfo_t *info, void *context)
{
	(void) signo;
	(void) info;
	(void) context;
	g_ack = 1;
}
	

int main(int argc, char **argv)
{
    struct sigaction sa;
    pid_t pid;
    int i;
    int len;
    unsigned char char_bit;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sa.sa_sigaction = receive_sig1;
    sigaction(SIGUSR1, &sa, NULL);

    /* 인자 처리(예외 처리 생략) */
    pid = (pid_t)atoi(argv[1]);
    len = 0;
    while (argv[2][len])
        len++;

    /* 문자열 비트 단위 전송 */
    i = -1;
    while (++i < len)
    {
        char_bit = (unsigned char)argv[2][i];
        send_char(char_bit, pid);
        
    }
    send_char('\n', pid);

    /* ACK 받은 뒤 */
    printf("\nconnect_done\n");
    return 0;
}

