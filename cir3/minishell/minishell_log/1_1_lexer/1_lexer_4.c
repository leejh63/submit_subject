/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   1_lexer_4.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 09:45:00 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/03 22:47:30 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "lexer.h"

static volatile sig_atomic_t	g_sig_int;
//-------------------------------------------------------------

size_t	ft_strlen(const char *str)
{
	size_t	i;

	i = 0;
	while (str[i] != '\0')
		i++;
	return (i);
}

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

void	*ft_memcpy(void *dest, const void *src, size_t n)
{
	size_t	i;

	if (!dest && !src)
		return (NULL);
	i = 0;
	while (i < n)
	{
		((unsigned char *)dest)[i] = ((unsigned char *)src)[i];
		i++;
	}
	return (dest);
}

char	*ft_strdup(const char *s)
{
	char	*dup;
	size_t	orilen;

	orilen = ft_strlen(s) + 1;
	dup = malloc(sizeof(char) * (orilen));
	if (!dup)
		return (0);
	return ((char *)ft_memcpy((void *)dup, (void *)s, orilen));
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
//-----------------------------------------------------------------------------------------
void	init_lexc(t_lex *lexc, char *cmd)
{
	lexc->rline = cmd;
	ft_memset(lexc->buf, 0, 1024);
	lexc->bind = 0;
	lexc->ind = 0;
	lexc->clen = ft_strlen(cmd);
}

void	fill_lex_buf(t_lex *lexc, int count)
{
	while (count--)
	{
		lexc->buf[lexc->bind] = lexc->rline[lexc->ind];
		lexc->bind++;
		lexc->ind++;
	}
}
//----리스트 출력
void	print_list(t_tk *one_tk)
{
	char	*e_char[] = {
		"none",
		"token_word",
		"token_pipe",
		"token_inrd",
		"token_heredoc",
		"token_outrd",
		"token_aprd",
		"token_env",
		"token_env2",
		"env_word",
		"token_eof",
		"token_eoc",
		"s_quotes",//
		"b_quotes"//
	};
	printf("my addr    =%p=\ntype:      =%s=\ncontent:   =%s=\nnext addr =%p=\n", one_tk, e_char[one_tk->w_type], one_tk->w_content, one_tk->next);
}

void	printall(t_tk *head)
{
	int i = 0;
	if (!head)
	{
		printf("\nNULL head!\n");
		return ;
	}
	printf("\n-------start_list--------\n");
	while (head)
	{
		printf("\ni : %d\n", i);
		print_list(head);
		head = head->next;
		i++;
	}
	printf("\n---------end_list--------\n");
}
//---------

void	free_all(t_tk *head)
{
	t_tk *start;

	start = head;
	while (start)
	{
		free(start->w_content);
		start = start->next;
		free(head);
		head = start;
	}
}

int	make_lex_onelist(t_tk **onelist)
{
	*onelist = malloc(sizeof(t_tk));
	if (!*onelist)
		return (1);
	(*onelist)->w_content = NULL;
	(*onelist)->w_type = none;
	(*onelist)->next = NULL;
	return (0);
}

t_tk	*move_to_end(t_tk *head)
{
	while (head->next)
		head = head->next;
	return (head);
}

int	lex_add_list(t_tk *head, t_lex *lexc, int w_type)
{
	t_tk	*end;
	t_tk	*onelist;

	if (head->w_content == none)
	{	
		head->w_type = w_type;
		head->w_content = ft_strdup(lexc->buf);
		if (!head->w_content)
			return (1);
	}
	else
	{
		if (make_lex_onelist(&onelist))
			return (1);
		end = move_to_end(head);
		onelist->w_type = w_type;
		onelist->w_content = ft_strdup(lexc->buf);
		if (!onelist->w_content)
			return (1);
		end->next = onelist;
	}
	return (0);
}


int	fill_token_list(t_lex *lexc, t_tk *head, int stat)
{
	if (stat == token_word)
		return (lex_add_list(head, lexc, stat));
	else if (stat == env_word)
		return (lex_add_list(head, lexc, stat));
	else if (stat == token_pipe)
		return (lex_add_list(head, lexc, stat));
	else if (stat == token_inrd)
		return (lex_add_list(head, lexc, stat));
	else if (stat == token_heredoc)
		return (lex_add_list(head, lexc, stat));
	else if (stat == token_outrd)
		return (lex_add_list(head, lexc, stat));
	else if (stat == token_aprd)
		return (lex_add_list(head, lexc, stat));
	else if (stat == token_env)
		return (lex_add_list(head, lexc, stat));
	else if (stat == env_word)
		return (lex_add_list(head, lexc, stat));
	else if (stat == token_env2)
		return (lex_add_list(head, lexc, stat));
	else if (stat == token_eoc)
		return (lex_add_list(head, lexc, stat));
	return (1);
}

int	fill_token_word(t_lex *lexc, t_tk *head, int stat)
{
	int	buf_len;

	lexc->buf[lexc->bind] = '\0';
	buf_len = ft_strlen(lexc->buf);
	if (buf_len)
	{
		if (fill_token_list(lexc, head, stat))
			return (1);
	}
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


//일단 시작이 대/소영어문자 혹은 _가 아닐경우 그냥 환경변수 포함 word 취급
// 또한 $"asdsa", $'abcd', (), [] 처리 관련 이야기가 없으므로 그냥 문자열처리 만약 필요하다면 여기함수 에서 수정진행
int	lexenv_func(t_lex *lexc, t_tk *head)
{
	if (fill_token_word(lexc, head, token_word))
		return (1);
	if (lexc->rline[lexc->ind + 1] == '?')
	{
		fill_lex_buf(lexc, 2);
		if (fill_token_word(lexc, head, token_env2))
			return (1);
	}
	else if (check_lexenv(lexc->rline[lexc->ind + 1], 1))
	{
		fill_lex_buf(lexc, 1);
		if (fill_token_word(lexc, head, token_env))
			return (1);
		while (check_lexenv(lexc->rline[lexc->ind], 0))
		{
			if (!lexc->rline[lexc->ind])
				return (1);
			fill_lex_buf(lexc, 1);
		}
		if(fill_token_word(lexc, head, env_word))
			return (1);
	}// 이쪽을 수정해야함
	else 
		fill_lex_buf(lexc, 1);
	return (0);
}

int	lexbquo_func(t_lex *lexc, t_tk *head)
{
	lexc->ind += 1;
	while (lexc->rline[lexc->ind] != '"')
	{
		if (!lexc->rline[lexc->ind])
			return (1);
		else if (lexc->rline[lexc->ind] == '$')
		{
			if (lexenv_func(lexc, head))
				return (1);
			if (lexc->rline[lexc->ind] == '"')
				break ;
		}
		fill_lex_buf(lexc, 1);
	}
	lexc->ind += 1;
	return (0);
}

int	lexsquo_func(t_lex *lexc, t_tk *head)
{
	(void) head;

	lexc->ind += 1;
	while (lexc->rline[lexc->ind] != '\'')
	{
		if (!lexc->rline[lexc->ind])
			return (1);
		fill_lex_buf(lexc, 1);
	}
	lexc->ind += 1;
	return (0);
}

int	lexleftrd_func(t_lex *lexc, t_tk *head)
{
	if (fill_token_word(lexc, head, token_word))
		return (1);
	if (lexc->rline[lexc->ind + 1] == '<')
	{
		fill_lex_buf(lexc, 2);
		if (fill_token_word(lexc, head, token_heredoc))
			return (1);
	}
	else
	{
		fill_lex_buf(lexc, 1);
		if (fill_token_word(lexc, head, token_inrd))
			return (1);
	}
	return (0);
}

int	lexrightrd_func(t_lex *lexc, t_tk *head)
{
	if (fill_token_word(lexc, head, token_word))
		return (1);
	if (lexc->rline[lexc->ind + 1] == '>')
	{
		fill_lex_buf(lexc, 2);
		if (fill_token_word(lexc, head, token_aprd))
			return (1);
	}
	else
	{
		fill_lex_buf(lexc, 1);
		if (fill_token_word(lexc, head, token_outrd))
			return (1);
	}
	return (0);
}

int	lexpipe_func(t_lex *lexc, t_tk *head)
{
	if (fill_token_word(lexc, head, token_word))
		return (1);
	fill_lex_buf(lexc, 1);
	if (fill_token_word(lexc, head, token_pipe))
		return (1);
	return (0);
}

int	lexspace_func(t_lex *lexc, t_tk *head)
{
	while (lexc->rline[lexc->ind] == ' ')
		lexc->ind++;
	if (fill_token_word(lexc, head, token_word))
		return (1);
	return (0);
}

int	lexing_func(t_lex *lexc, t_tk *head)
{
	(void) head;

	fill_lex_buf(lexc, 1);
	return (0);
}

void	lex_check_func(int (**lex_func_ptr)(t_lex *, t_tk *), t_lex *lexc)
{
	if (lexc->rline[lexc->ind] == '"')
		*lex_func_ptr = lexbquo_func;
	else if (lexc->rline[lexc->ind] == '\'')
		*lex_func_ptr = lexsquo_func;
	else if (lexc->rline[lexc->ind] == '<')
		*lex_func_ptr = lexleftrd_func;
	else if (lexc->rline[lexc->ind] == '>')
		*lex_func_ptr = lexrightrd_func;
	else if (lexc->rline[lexc->ind] == '|')
		*lex_func_ptr = lexpipe_func;
	else if (lexc->rline[lexc->ind] == '$')
		*lex_func_ptr = lexenv_func;
	else if (lexc->rline[lexc->ind] == ' ')
		*lex_func_ptr = lexspace_func;
	else
		*lex_func_ptr = lexing_func;
}

int	change_word(char *test, t_tk **lex_list)
{
	t_lex	lexc;
	int (*lex_func_ptr)(t_lex *, t_tk *);
	t_tk	*list_head;

	init_lexc(&lexc, test);
	if (make_lex_onelist(&list_head))
		return (1);
	while (lexc.rline[lexc.ind])
	{
		lex_check_func(&lex_func_ptr, &lexc);
		if (lex_func_ptr(&lexc, list_head))
			return (free_all(list_head), 1);
	}
	if (fill_token_word(&lexc, list_head, token_word))
		return (free_all(list_head), 1);
	ft_memcpy(&lexc.buf, "EOC", 3);
	lexc.bind = 3;
	if (fill_token_word(&lexc, list_head, token_eoc))
		return (free_all(list_head), 1);
	*lex_list = list_head;
	return (0);
}
//------------------------------------------------------------
void	ctrl_c(void)
{
	write(1, "\n", 1);
	rl_replace_line("", 0);
	rl_on_new_line();
	rl_redisplay();
	g_sig_int = 0;
}

void	sig_int_func(int sig_no, siginfo_t *info, void *ctx)
{
	(void) info;
	(void) ctx;

	g_sig_int = sig_no;
	if (g_sig_int == SIGINT)
		ctrl_c();
}

int	signal_set(void)
{
	struct sigaction	sig;

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
	t_tk	*lex_list;
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
				if (change_word(read_line, &lex_list))
				{
					printf(" after: wrong quote!\n");
					free_all(lex_list);
				}
				else
				{
					printf("goood\n");
					printall(lex_list);
					free_all(lex_list);
				}
			}
		}
		free(read_line);
	}
}
