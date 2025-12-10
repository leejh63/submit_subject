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

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/*
'a'의 비트는 01100001입니다
'y'의 비트는 01111001은 문자 
*/
void	send_char(unsigned char message, pid_t pid)
{
	int		i;

	i = 8;
	while (i--)
	{
		if (message & (1 << i))
			kill(pid, SIGUSR2);
		else
			kill(pid, SIGUSR1);
		usleep(20);
	}
}

int	main(int argc, char **argv)
{
	pid_t	pid;	
	int 	sig;
	int	i;
	int	len;
	unsigned char	char_bit;
	
	len = 0;
	while (argv[2][len])
		len++;
	pid = (pid_t)atoi(argv[1]);
	i = -1;
	while (++i < len)
	{
		char_bit = argv[2][i];
		send_char(char_bit, pid);
	}
	send_char('\n', pid);
	printf("\nconnect_done\n");
	return (0);
}
