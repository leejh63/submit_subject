/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   basic1.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 09:45:00 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/02 09:45:02 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lexer.h"

static volatile sig_atomic_t sig_int;
//-------------------------------------------------------------
void	*ft_memset(void *s, int c, size_t n)
{
	size_t	i;
	size_t	uc;

	i = 0;
	uc = (unsigned char)c;
	while (i < n)
	{
		((unsigned char *)s)[i] = uc;
		i++;
	}
	return (s);
}

size_t	ft_strlen(const char *str)
{
	size_t	i;

	i = 0;
	while (str[i] != '\0')
		i++;
	return (i);
}

void	*ft_memmove(void *dest, const void *src, size_t n)
{
	size_t	i;

	if (!dest && !src)
		return (NULL);
	if (dest < src)
	{
		i = 0;
		while (i < n)
		{
			((unsigned char *)dest)[i] = ((unsigned char *)src)[i];
			i++;
		}
	}
	else
	{
		while (n > 0)
		{
			((unsigned char *)dest)[n - 1] = ((unsigned char *)src)[n - 1];
			n--;
		}
	}
	return (dest);
}

int	change_word(char *test)
{
	int	i;
	int 	start;
	int	s_quo;
	int	slen;
	int	buf_ind;
	char	tmp_buf[1024];

	i = 0;
	s_quo = 0;
	slen = ft_strlen(test);
	buf_ind = 0;
	ft_memset(tmp_buf, 0, 1024);
	// 처리는 bash 기준
	while (test[i])
	{
		if (test[i] == 92 && test[i + 1])	// / 처리
		{
			ft_memmove(&test[i], &test[i + 1], slen - i);
			tmp_buf[buf_ind] = test[i];
			buf_ind++;
			i += 1;
			continue ;
		}
		else if (test[i] == 34)	// " 처리
		{
			ft_memmove(&test[i], &test[i + 1], slen - i);
			while (test[i] != 34)
			{
				if (!test[i])
					return (1);
				if (test[i] == 92)
					ft_memmove(&test[i], &test[i + 1], slen - i);
				tmp_buf[buf_ind] = test[i];
				buf_ind++;
				i++;
			}
			ft_memmove(&test[i], &test[i + 1], slen - i);
		}
		else if (test[i] == 39)	// ' 처리
		{
			ft_memmove(&test[i], &test[i + 1], slen - i);
			while (test[i] != 39)
			{
				if (!test[i])
					return (1);
				tmp_buf[buf_ind] = test[i];
				buf_ind++;
				i++;
			}
			ft_memmove(&test[i], &test[i + 1], slen - i);
		}
		else if (test[i] == '<')
		{
			tmp_buf[buf_ind] = '\0';
			if (ft_strlen(tmp_buf)) // sort >out 문제 발생 추가코드
				printf("1word -=-%s-=-   len: %zu\n", tmp_buf, ft_strlen(tmp_buf));
			ft_memset(tmp_buf, 0, 1024);
			buf_ind = 0;
			if (test[i + 1] == '<')
			{
				printf("1re << -=-%s-=-   len: %d\n", "<<", 2);
				i += 2;
			}
			else
			{
				printf("1re < -=-%s-=-   len: %d\n", "<", 1);
				i += 1;
			}
		}
		else if (test[i] == '>')
		{
			tmp_buf[buf_ind] = '\0';
			if (ft_strlen(tmp_buf))
				printf("2word -=-%s-=-   len: %zu\n", tmp_buf, ft_strlen(tmp_buf));
			ft_memset(tmp_buf, 0, 1024);
			buf_ind = 0;
			if (test[i + 1] == '>')
			{
				printf("2re >> -=-%s-=-   len: %d\n", ">>", 2);
				i += 2;
			}
			else
			{
				printf("2re > -=-%s-=-   len: %d\n", ">", 1);
				i += 1;
			}
		}
		else if (test[i] == '|')
		{
			tmp_buf[buf_ind] = '\0'; 
			printf("3word -=-%s-=-   len: %zu\n", tmp_buf, ft_strlen(tmp_buf));
			ft_memset(tmp_buf, 0, 1024);
			buf_ind = 0;
			printf("1pipe | -=-%s-=-   len: %d\n", "|", 1);
			i += 1;
		}
		else if (test[i] == 32) // 공백
		{
			while (test[i] == 32)
				i++;
			tmp_buf[buf_ind] = '\0';
			if (ft_strlen(tmp_buf))
				printf("4word -=-%s-=-   len: %zu\n", tmp_buf, ft_strlen(tmp_buf));
			ft_memset(tmp_buf, 0, 1024);
			buf_ind = 0;
		}
		else
		{
			tmp_buf[buf_ind] = test[i];
			buf_ind++;
			i++;
		}
		
	}
	tmp_buf[buf_ind] = '\0'; 
	printf("5word -=-%s-=-   len: %zu\n", tmp_buf, ft_strlen(tmp_buf));
	ft_memset(tmp_buf, 0, 1024);
	buf_ind = 0;
	
	printf("EOC -=-%s-=-   len: %d\n", "EOC", 1);
	return (0);
}
//------------------------------------------------------------
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
	char	**f_split;

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
				add_history(read_line);
				printf("before: %s\n", read_line);
				if (change_word(read_line))
					printf(" after: wrong quote!\n");
				else
					printf(" after: %s\n", read_line);	
			}
		}
		free(read_line);
	}
}
