/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   1_lexer_3.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/03 19:04:22 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/03 19:04:23 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//  역슬래시 주석처리만 진행함..

#include "lexer.h"

static volatile sig_atomic_t sig_int;
//-------------------------------------------------------------

int	ft_isupper(int str)
{
	unsigned char	unstr;

	if (str < 0 || str > 255)
		return (0);
	unstr = (unsigned char)str;
	return (unstr <= 'Z' && unstr >= 'A');
}

int	ft_islower(int str)
{
	unsigned char	unstr;

	if (str < 0 || str > 255)
		return (0);
	unstr = (unsigned char)str;
	return (unstr <= 'z' && unstr >= 'a');
}

int	ft_isalpha(int str)
{
	return (ft_isupper(str) || ft_islower(str));
}

int	ft_isdigit(int str)
{
	unsigned char	unstr;

	if (str < 0 || str > 255)
		return (0);
	unstr = (unsigned char)str;
	return (unstr >= '0' && unstr <= '9');
}

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

void	init_lexc(t_lex *lexc, char *cmd)
{
	lexc->rline = cmd;
	ft_memset(lexc->buf, 0, 1024);
	lexc->bind = 0;
	lexc->ind = 0;
	lexc->clen = ft_strlen(cmd);
}

void	fill_lex_buf(t_lex *lexc)
{
	lexc->buf[lexc->bind] = lexc->rline[lexc->ind];
	lexc->bind++;
	lexc->ind++;
}

int	fill_token_list(t_lex *lexc, int tmp_arg, int stat)
{
	if (stat == token_word)
		return (!printf("\nword     -=-%s-=-   len: %d    arg %d \n", lexc->buf, tmp_arg, stat));
	else if (stat == env_word)
		return (!printf("\nenv_word -=-%s-=-   len: %d    arg %d \n", lexc->buf, tmp_arg, stat));
	else if (stat == token_pipe)
		return (!printf("\npipe |   -=-%s-=-   len: %d\n", "|", 1));
	else if (stat == token_inrd)
		return (!printf("\nre   <   -=-%s-=-   len: %d\n", "<", 1));
	else if (stat == token_heredoc)
		return (!printf("\nre   <<  -=-%s-=-   len: %d\n", "<<", 2)); 
	else if (stat == token_outrd)
		return (!printf("\nre   >   -=-%s-=-   len: %d\n", ">", 1));
	else if (stat == token_aprd)
		return (!printf("\nre   >>  -=-%s-=-   len: %d\n", ">>", 2));
	else if (stat == token_env)
		return (!printf("\nenv1 $   -=-%s-=-   len: %d\n", "$", 1));
	else if (stat == env_word)
		return (!printf("\nenv_word -=-%s-=-   len: %d\n", "$", 1));
	else if (stat == token_env2)
		return (!printf("\nenv2 $?  -=-%s-=-   len: %d\n", "$?", 2));
	else if (stat == token_eoc)
		return (!printf("\nEOC      -=-%s-=-   len: %d\n", "EOC", 1));
	return (1);
}

int	fill_token_word(t_lex *lexc, int tmp_arg, int stat)// tmp_arg에 리스트 구조체 들어갈 예정
{
	int	buf_len;

	lexc->buf[lexc->bind] = '\0';
	buf_len = ft_strlen(lexc->buf);
	if (buf_len)
		fill_token_list(lexc, buf_len, stat);
	ft_memset(lexc->buf, 0, 1024);
	lexc->bind = 0;
	return (0);
}

int	check_lexenv(char envstr, int start)
{
	if (start)
	{
		if (ft_isalpha(envstr) || envstr == '_')
			return (1);
	}
	else
	{
		if (ft_isalpha(envstr) || envstr == '_' || ft_isdigit(envstr))
			return (1);
	}
	return (0);
}

int	lexenv_func(t_lex *lexc)
{
	if (fill_token_word(lexc, ft_strlen(lexc->buf), token_word))
		return (1);
	if (lexc->rline[lexc->ind + 1] == '?')
	{
		if (fill_token_list(lexc, 1, token_env2))
			return (1);
		lexc->ind += 2;
	}
	else if (check_lexenv(lexc->rline[lexc->ind + 1], 1))
	{
		if (fill_token_list(lexc, 1, token_env))
			return (1);
		lexc->ind += 1;
		while (check_lexenv(lexc->rline[lexc->ind], 0))
		{
			if (!lexc->rline[lexc->ind])
				return (1);
			fill_lex_buf(lexc);
		}
		if(fill_token_word(lexc, ft_strlen(lexc->buf), env_word))
			return (1);
	}
//일단 시작이 대/소영어문자 혹은 _가 아닐경우 그냥 환경변수 포함 word 취급
// 또한 $"asdsa", $'abcd', (), [] 처리 관련 이야기가 없으므로 그냥 문자열처리 만약 필요하다면 여기에 추기 수정
	else 
		fill_lex_buf(lexc);
	return (0);
}

int	change_word(char *test)
{
	t_lex	lexc;

	init_lexc(&lexc, test);
	while (lexc.rline[lexc.ind])
	{
		/*
		if (lexc.rline[lexc.ind] == '\\' && lexc.rline[lexc.ind + 1])	// \ 처리
		{
			lexc.ind += 1;
			fill_lex_buf(&lexc);
			continue ;
		}*/
		if (lexc.rline[lexc.ind] == '"')
		{
			lexc.ind += 1;
			while (lexc.rline[lexc.ind] != '"')
			{
				if (!lexc.rline[lexc.ind])
					return (1);
				//if (lexc.rline[lexc.ind] == '\\')
					//lexc.ind += 1;
				else if (lexc.rline[lexc.ind] == '$')
				{
					if (lexenv_func(&lexc))
						return (1);
					if (lexc.rline[lexc.ind] == '"')
						break ;
					//else if (lexc.rline[lexc.ind] == '\\')
						//lexc.ind += 1;
				}
				fill_lex_buf(&lexc);
			}
			lexc.ind += 1;
		}
		else if (lexc.rline[lexc.ind] == '\'')
		{
			lexc.ind += 1;
			while (lexc.rline[lexc.ind] != '\'')
			{
				if (!lexc.rline[lexc.ind])
					return (1);
				fill_lex_buf(&lexc);
			}
			lexc.ind += 1;
		}
		else if (lexc.rline[lexc.ind] == '<')
		{
			if(fill_token_word(&lexc, ft_strlen(lexc.buf), token_word))
				return (1);
			if (lexc.rline[lexc.ind + 1] == '<')
			{
				if (fill_token_list(&lexc, 1, token_heredoc))
					return (1);
				lexc.ind += 2;
			}
			else
			{
				if (fill_token_list(&lexc, 1, token_inrd))
					return (1);
				lexc.ind += 1;
			}
		}
		else if (lexc.rline[lexc.ind] == '>')
		{
			if(fill_token_word(&lexc, ft_strlen(lexc.buf), token_word))
				return (1);
			if (lexc.rline[lexc.ind + 1] == '>')
			{
				if (fill_token_list(&lexc, 1, token_aprd))
					return (1);
				lexc.ind += 2;
			}
			else
			{
				if (fill_token_list(&lexc, 1, token_outrd))
					return (1);
				lexc.ind += 1;
			}
		}
		else if (lexc.rline[lexc.ind] == '|')
		{
			if(fill_token_word(&lexc, ft_strlen(lexc.buf), token_word))
				return (1);
			if (fill_token_list(&lexc, 1, token_pipe))
				return (1);
			lexc.ind += 1;
		}
		else if (lexc.rline[lexc.ind] == '$')
		{
			if (lexenv_func(&lexc))
				return (1);
		}
		else if (lexc.rline[lexc.ind] == ' ') // 공백
		{
			while (lexc.rline[lexc.ind] == ' ')
				lexc.ind++;
			if(fill_token_word(&lexc, ft_strlen(lexc.buf), token_word))
				return (1);
		}
		else // 일반 문자
		{
			fill_lex_buf(&lexc);
		}
		
	}
	if(fill_token_word(&lexc, ft_strlen(lexc.buf), token_word))
		return (1);
	if (fill_token_list(&lexc, 1, token_eoc))
		return (1);
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
					printf("goood\n");
			}
		}
		free(read_line);
	}
}
