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

#include "lexer.h"

//----------------tmp--------------------------
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
		"quot_word",
		"quotes",
		"space",
	};
//----------------------------------------------------------


static volatile sig_atomic_t	g_sig_int;

//-------------------------------------ft func------------------------

size_t	ft_strlen(const char *str)
{
	size_t	i;

	i = 0;
	while (str[i] != '\0')
		i++;
	return (i);
}

void	ft_putstr_fd(char *s, int fd)
{
	if (!s)
		return ;
	write(fd, s, ft_strlen(s));
	s++;
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
	return (s); // 사실 memset도 에러처리 해야함 기존s 와 반환 s 값이 달라져는가?
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
		return (perror("malloc(ft_strdup)"), NULL);
	return ((char *)ft_memcpy((void *)dup, (void *)s, orilen));
}

char	*ft_strndup(const char *s, size_t n)
{
	size_t len;
	char *dup;

	len = 0;
	while (len < n && s[len])
		len++;
	dup = malloc(len + 1);
	if (!dup)
	{
		perror("malloc(ft_strndup)");
		return NULL;
	}
	ft_memcpy(dup, s, len);
	dup[len] = '\0';
	return (dup);
}

size_t	ft_strlcpy(char *dst, const char *src, size_t size)
{
	size_t	slen;
	size_t	i;

	slen = ft_strlen(src);
	if (!size)
		return (slen);
	i = 0;
	while ((i < size - 1) && src[i])
	{
		dst[i] = src[i];
		i++;
	}
	dst[i] = '\0';
	return (slen);
}

size_t	ft_strlcat(char *dst, const char *src, size_t size)
{
	size_t	j;
	size_t	dsize;
	size_t	ssize;

	ssize = ft_strlen(src);
	if (!size)
		return (ssize);
	dsize = ft_strlen(dst);
	if (dsize >= size)
		return (size + ssize);
	j = 0;
	while (src[j] && ((dsize + j) < (size - 1)))
	{
		dst[dsize + j] = src[j];
		j++;
	}
	dst[dsize + j] = '\0';
	return (dsize + ssize);
}

int	ft_strcmp(const char *s1, const char *s2)
{
	size_t	i;
	size_t	len;
	size_t	ft_len1;
	size_t	ft_len2;

	i = 0;
	ft_len1 = ft_strlen(s1);
	ft_len2 = ft_strlen(s2);
	if (ft_len1 > ft_len2)
		len = ft_len1;
	else
		len = ft_len2;
	while (i < len)
	{
		if (!s1[i] || s1[i] != s2[i])
			return ((unsigned char)s1[i] - (unsigned char)s2[i]);
		i++;
	}
	return (0);
}

char	*ft_strjoin(char *s1, char const *s2)
{
	size_t	s1len;
	size_t	s2len;
	char	*newstr;

	if (!s1 || !s2)
		return (NULL);
	s1len = ft_strlen(s1);
	s2len = ft_strlen(s2);
	newstr = malloc(sizeof(char) * (s1len + s2len + 1));
	if (!newstr)
		return (perror("malloc(ft_strjoin)"), NULL);
	ft_strlcpy(newstr, s1, s1len + 1);
	ft_strlcat(newstr, s2, s1len + s2len + 1);
	return (newstr);
}

int	ft_strncmp(const char *s1, const char *s2, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		if (!s1[i] || s1[i] != s2[i])
			return ((unsigned char)s1[i] - (unsigned char)s2[i]);
		i++;
	}
	return (0);
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

//-------------------------------------ft func------------------------
//-----------------------single_list
//void	move_to end_slint(void *)
//-------------------------------------------

//----리스트 출력
void	print_list(t_tk *one_tk, char *readline)
{
	printf("my addr    =%p=\ntype:      =%s=\ncontent:   =%s=\nnext addr  =%p=\n", one_tk, e_char[one_tk->w_type], one_tk->w_content, one_tk->next);
	printf("cmd: %s\n", readline);
}

void	print_argv_redirs(t_par *head)
{
	t_cmd *argv = head->argvs;
	t_redir *redirs = head->redirs;
	char left[64], right[64];
while (argv || redirs)
    {
        // ── 주소 열 ─────────────────────────────────────────────────────────────
        if (argv)
            (void)snprintf(left, sizeof left, "addr: %p", (void*)argv);
        else
            left[0] = '\0';

        if (redirs)
            (void)snprintf(right, sizeof right, "addr: %p", (void*)redirs);
        else
            right[0] = '\0';

        printf("%-40s | %-40s\n", left, right);

        // ── 타입 열 ─────────────────────────────────────────────────────────────
        if (argv)
            (void)snprintf(left, sizeof left, "type: %s", e_char[argv->w_type]);
        else
            left[0] = '\0';

        if (redirs)
            (void)snprintf(right, sizeof right, "type: %s", e_char[redirs->r_type]);
        else
            right[0] = '\0';

        printf("%-40s | %-40s\n", left, right);

        // ── 단어/파일 열 ─────────────────────────────────────────────────────────
        if (argv)
            (void)snprintf(left, sizeof left, "word: =%s=",
                           argv->cmd ? argv->cmd : "(null)");
        else
            left[0] = '\0';

        if (redirs)
            (void)snprintf(right, sizeof right, "r_word: =%s=",
                           redirs->r_word ? redirs->r_word : "(null)");
        else
            right[0] = '\0';

        printf("%-40s | %-40s\n", left, right);

        // ── next 포인터 열 ────────────────────────────────────────────────────────
        if (argv)
            (void)snprintf(left, sizeof left, "next: %p", (void*)argv->next);
        else
            left[0] = '\0';

        if (redirs)
            (void)snprintf(right, sizeof right, "next: %p", (void*)redirs->next);
        else
            right[0] = '\0';
        if (redirs)
	  	(void)snprintf(right, sizeof right, "heredoc_fd: %d", redirs->heredoc_fd);
        printf("%-40s | %-40s\n\n", left, right);

        if (argv)   argv   = argv->next;
        if (redirs) redirs = redirs->next;
    }
	
}

void	print_par_list(t_par *head)
{
	int i = 0;
	if (!head)
	{
		printf("\nNULL   par   list!\n");
		return ;
	}
	printf("\n-------start  par   list--------\n");
	while (head)
	{
		printf("\n=====================pipe : %d===================================\n\n", i);
		print_argv_redirs(head);
		printf("===================================================================\n");
		head = head->next;
		i++;
	}
	printf("\n--------end   par    list--------\n");
}

void	print_lex(t_tk *head, char *readline)
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
		print_list(head, readline);
		head = head->next;
		i++;
	}
	printf("\n---------end_list--------\n");
}

void	print_env(t_env *head)
{
	int	i = 0;
	printf("========== start env_list ===========\n");
	if (!head)
	{
		printf("\nNULL head!\n");
		return ;
	}
	while (head)
	{
		printf("i = %d\n", i);
		printf("env_name     : (%s)\n", head->env_name);
		printf("env_content  : (%s)\n", head->env_content);
		i++;
		head = head->next;
	}
	printf("========== end env_list ===========\n\n");
}

//---------
//-------------------------error
int	make_func_err(errnum)
{
	if (errnum == e_squo)
		ft_putstr_fd("syntax error: \' \n", 2);
	else if (errnum == e_bquo)
		ft_putstr_fd("syntax error: \" \n", 2);
	else if (errnum == e_pipe)
		ft_putstr_fd("syntax error: | \n", 2);
	else if (errnum == e_inrd)
		ft_putstr_fd("syntax error: < \n", 2);
	else if (errnum == e_outrd)
		ft_putstr_fd("syntax error: > \n", 2);
	else if (errnum == e_aprd)
		ft_putstr_fd("syntax error: >> \n", 2);
	else if (errnum == e_heredoc)
		ft_putstr_fd("syntax error: << \n", 2);
	else if (errnum == e_env)
		ft_putstr_fd("syntax error: $ \n", 2);
	return (1);
}
//-------------------------error

//-------------------------lexer

void	fill_lex_buf(t_lex *lexc, int count)
{
	while (count--)
	{
		lexc->buf[lexc->bind] = lexc->rline[lexc->ind];
		lexc->bind++;
		lexc->ind++;
	}
}

void	free_all_lex(t_tk **head)
{
	t_tk *start;

	start = *head;
	while (start)
	{
		if (start->w_content)
			free(start->w_content);
		start->w_content = NULL;
		start = start->next;
		free(*head);
		*head = start;
	}
	*head = NULL;
}

int	make_lex_onelist(t_tk **onelist)
{
	*onelist = malloc(sizeof(t_tk));
	if (!*onelist)
		return (perror("malloc(make_lex_onelist)"), 1);
	(*onelist)->w_content = NULL;
	(*onelist)->w_type = none;
	(*onelist)->next = NULL;
	return (0);
}

int	init_lexc(t_lex *lexc, char *cmd, t_tk **list_head)
{
	lexc->rline = cmd;
	ft_memset(lexc->buf, 0, TMPBUF);
	lexc->bind = 0;
	lexc->ind = 0;
	lexc->clen = ft_strlen(cmd);
	lexc->clen = token_word;
	if (make_lex_onelist(list_head))
		return (1);
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

int	check_meta(char chr)
{
	int	meta_ind;
	char	*meta_set;

	meta_set = " \"'|<>$"; //\t\n\v\f\r  일단 공백만 화이트 스페이스는 일단 보류
	
	meta_ind = 0;
	while (meta_ind < 7)
	{
		if (!chr || meta_set[meta_ind] == chr)
			return (0);
		meta_ind++;
	}
	return (1);
}

int	fill_token_word(t_lex *lexc, t_tk *head, int stat)
{
	int	buf_len;

	lexc->buf[lexc->bind] = '\0';
	buf_len = ft_strlen(lexc->buf);
	if (buf_len)
	{
		if (lex_add_list(head, lexc, stat))
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

int	lexing_func(t_lex *lexc, t_tk *head)
{
	while (check_meta(lexc->rline[lexc->ind]))
		fill_lex_buf(lexc, 1);
	if (lexc->rline[lexc->ind] != '$')
		if (fill_token_word(lexc, head, lexc->stat))
			return (1);
	return (0);
}

int	lex_env_norm(t_lex *lexc, t_tk *head)
{
	if (fill_token_word(lexc, head, lexc->stat))
		return (1);
	fill_lex_buf(lexc, 1);
	//if(fill_token_word(lexc, head, token_env))
	//	return (1);
	while (check_lexenv(lexc->rline[lexc->ind], 0))
		fill_lex_buf(lexc, 1);
	if(fill_token_word(lexc, head, env_word))
		return (1);
	return (0);
}

int	lexenv_func(t_lex *lexc, t_tk *head)
{
	if (lexc->rline[lexc->ind + 1] == '?')
	{
		if (fill_token_word(lexc, head, lexc->stat))
			return (1);
		fill_lex_buf(lexc, 2);
		if (fill_token_word(lexc, head, token_env2))
			return (1);
		return (0);
	}
	else if (check_lexenv(lexc->rline[lexc->ind + 1], 1))
	{
		return (lex_env_norm(lexc, head));
	}
	fill_lex_buf(lexc, 1);
	return (0);
}

int	lex_bquo_check(t_lex *lexc, t_tk *head)
{
	(void) head;
	fill_lex_buf(lexc, 1);
	while (lexc->rline[lexc->ind] != '"')
	{
		if (!lexc->rline[lexc->ind])
			return (make_func_err(e_bquo));
/*		else if (lexc->rline[lexc->ind] == '$')
		{
			if (lexenv_func(lexc, head))
				return (1);
			if (lexc->rline[lexc->ind] == '"')
				return (0);
		}*/
		else
			fill_lex_buf(lexc, 1);
	}
	fill_lex_buf(lexc, 1);
	return (0);
}

int	lexbquo_func(t_lex *lexc, t_tk *head)
{
	if (lexc->rline[lexc->ind + 1] == '"')
	{
		lexc->ind += 2;
		return (0);
	}
	if (fill_token_word(lexc, head, lexc->stat))
		return (1);
	lexc->stat = quotes;
	if (lex_bquo_check(lexc, head))
		return (1);
	if (fill_token_word(lexc, head, lexc->stat))
			return (1);
	return (0);
}

int	lexsquo_func(t_lex *lexc, t_tk *head)
{
	(void) head;
	int	stat;

	stat = token_word;
	fill_lex_buf(lexc, 1);
	while (lexc->rline[lexc->ind] != '\'')
	{
		if (!lexc->rline[lexc->ind])
			return (make_func_err(e_squo));
		fill_lex_buf(lexc, 1);
	}
	fill_lex_buf(lexc, 1);
	if (fill_token_word(lexc, head, stat))
			return (1);
	return (0);
}

int	lexleftrd_func(t_lex *lexc, t_tk *head)
{
	int	stat;

	if (fill_token_word(lexc, head, lexc->stat))
		return (1);
	if (lexc->rline[lexc->ind + 1] == '<')
	{
		fill_lex_buf(lexc, 2);
		stat = token_heredoc;
	}
	else
	{
		fill_lex_buf(lexc, 1);
		stat = token_inrd;
	}
	if (fill_token_word(lexc, head, stat))
		return (1);
	return (0);
}

int	lexrightrd_func(t_lex *lexc, t_tk *head)
{
	int	stat;

	if (fill_token_word(lexc, head, lexc->stat))
		return (1);
	if (lexc->rline[lexc->ind + 1] == '>')
	{
		fill_lex_buf(lexc, 2);
		stat = token_aprd;
	}
	else
	{
		fill_lex_buf(lexc, 1);
		stat = token_outrd;

	}
	if (fill_token_word(lexc, head, stat))
		return (1);
	return (0);
}

int	lexpipe_func(t_lex *lexc, t_tk *head)
{
	int	stat;

	if (fill_token_word(lexc, head, lexc->stat))
		return (1);
	stat = token_pipe;
	fill_lex_buf(lexc, 1);
	if (fill_token_word(lexc, head, stat))
		return (1);
	return (0);
}

int	lexspace_func(t_lex *lexc, t_tk *head)
{
	int	stat;

	stat = token_word;
	if (fill_token_word(lexc, head, stat))
		return (1);
	while (lexc->rline[lexc->ind] == ' ')
		lexc->ind++;
	ft_memcpy(lexc->buf, " ", 1);
	lexc->bind += 1;
	if (fill_token_word(lexc, head, space))
		return (1);
	return (0);
}

int	lex_eoc_func(t_lex *lexc, t_tk *head)
{
	if (fill_token_word(lexc, head, lexc->stat))
		return (1);
	ft_memcpy(lexc->buf, "EOC", 3);
	lexc->bind = 3;
	if (fill_token_word(lexc, head, token_eoc))
		return (1);
	return (0);
}

void	lex_check_func(int (**lex_func_ptr)(t_lex *, t_tk *), t_lex *lexc)
{
	lexc->stat = token_word;
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

int	lexer(t_bundle *bundle)
{
	t_lex	lexc;
	int (*lex_func_ptr)(t_lex *, t_tk *);
	t_tk	*list_head;

	if (init_lexc(&lexc, bundle->read_line, &list_head))
		return (1);
	while (lexc.rline[lexc.ind])
	{
		lex_check_func(&lex_func_ptr, &lexc);
		if (lex_func_ptr(&lexc, list_head))
			return (free_all_lex(&list_head), 1);
	}
	if (lex_eoc_func(&lexc, list_head))
		return (free_all_lex(&list_head), 1);
	bundle->lex_list = list_head;
	return (0);
}
//-------------------------lexer

//------------------------paser


void	clean_all_argv(t_cmd *argvs)
{
	t_cmd *start;

	start = argvs;
	while (start)
	{
		if (start->cmd)
			free(start->cmd);
		start->cmd = NULL;
		start = start->next;
		free(argvs);
		argvs = start;
	}
}

void	clean_all_redir(t_redir *redirs)
{
	t_redir *start;

	start = redirs;
	while (start)
	{
		if (redirs->r_word)
			free(redirs->r_word);
		start->r_word = NULL;
		if (start->heredoc_fd != -1)
			close(start->heredoc_fd);
		start = start->next;
		free(redirs);
		redirs = start;
	}
}

void	free_all_par(t_par **head)
{
	t_par *start;

	start = *head;
	while (start)
	{
		clean_all_argv((*head)->argvs);
		clean_all_redir((*head)->redirs);
		start = start->next;
		free(*head);
		*head = start;
	}
	*head = NULL;
}

int	init_one_argv(t_cmd **argv_cmd)
{
	(*argv_cmd) = malloc(sizeof(t_cmd));
	if (!(*argv_cmd))
		return (perror("malloc(init_one_argv)"), 1);
	(*argv_cmd)->next = NULL;
	(*argv_cmd)->cmd = NULL;
	(*argv_cmd)->w_type = none;
	return (0);
}



int	init_one_redir(t_redir **redir)
{
	(*redir) = malloc(sizeof(t_redir));
	if (!(*redir))
		return (perror("malloc(init_one_redir)"), 1);
	(*redir)->next = NULL;
	(*redir)->r_word = NULL;
	(*redir)->r_type = none;
	(*redir)->heredoc_fd = -1;
	return (0);
}

int	init_par_list(t_par **par_list)
{
	static int	ind;

	*par_list = malloc(sizeof(t_par));
	if (!*par_list)
		return (perror("malloc(init_par_list)"), 1);
	(*par_list)->tmp_num = ind;
	ind++;
	if (init_one_argv(&(*par_list)->argvs))
		return (1);
	if (init_one_redir(&(*par_list)->redirs))
		return (1);
	(*par_list)->next = NULL;
	(*par_list)->previous_type = none;
	return (0);
}
//===========================env_list===========================
// 타입이 env_word, quot_word, token_env2, 리다이렉션 시 무조건 환경변수 확인진행

int	find_meta(char *env, char meta)
{
	int	i;

	i = 0;
	while (env[i])
	{
		if (env[i] == meta)
		{
			env[i] = '\0';
			return (i);
		}
		i++;
	}
	return (-1);
}

int	fill_env_name(t_env *new_list, char *env)
{
	new_list->env_name = ft_strdup(env);
	if (!new_list->env_name)
		return (1);
	return (0);
}

int	fill_env_content(t_env *new_list, char *env, int equal_ind)
{
	if (equal_ind == -1)
		new_list->env_content = NULL;
	else
	{
		new_list->env_content = ft_strdup(&env[equal_ind + 1]);
		if (!new_list->env_content)
			return (1);
	}
	return (0);
}

int	set_env_newlist(t_env **new_list, char *env)
{
	int	equal_ind;
	char	*tmp_env;

	(*new_list) = malloc(sizeof(t_env));
	if (!*new_list)
		return (perror("malloc(init_env_list)"), 1);
	tmp_env = ft_strdup(env);
	if (!tmp_env)
		return (1);
	equal_ind = find_meta(tmp_env, '=');
	if (fill_env_name(*new_list, tmp_env))
		return (free(tmp_env), 1);
	if (fill_env_content(*new_list, tmp_env, equal_ind))
		return (free(tmp_env), 1);
	(*new_list)->prev = NULL;
	(*new_list)->next = NULL;
	return (free(tmp_env), 0);
}

void	add_front_env_list(t_env **env_list, t_env *new_env)
{
	new_env->next = *env_list;
	(*env_list)->prev = new_env;
	(*env_list) = new_env;
}

int	tmp_add_env_list(t_env **env_list, char *env)// env 테스트용
{
	t_env	*new_env;

	if (set_env_newlist(&new_env, env))
		return (1);
	if (!*env_list)
		*env_list = new_env;
	else
		add_front_env_list(env_list, new_env);
	return (0);
}

t_env	*find_env_name(t_env *env_list, char *name)
{
	while (env_list)
	{
		if (!ft_strcmp(env_list->env_name, name))
			return (env_list);
		env_list = env_list->next;
	}
	return (NULL);
}

void	free_env_one(t_env *del_one)
{
	if (!del_one)
		return ;
	if (del_one->env_name)
		free(del_one->env_name);
	if (del_one->env_content)
		free(del_one->env_content);
	free(del_one);
}

int	del_one_env_list(t_env **env_list, char *del_name)
{
	t_env	*del_env;
	t_env	*del_prev;
	t_env	*del_next;


	del_env = find_env_name(*env_list, del_name);
	if (!del_env)
		return (1);
	del_prev = del_env->prev;
	del_next = del_env->next;
	free_env_one(del_env);
	if (!del_prev)
	{
		(*env_list) = del_next;
		if (del_next)
			(*env_list)->prev = NULL;
	}
	else if (!del_next)
		del_prev->next = NULL;
	else
	{
		del_prev->next = del_next;
		del_next->prev = del_prev;
	}
	return (0);
}

void	free_env(t_env *env_list)
{
	t_env	*head;

	while (env_list)
	{
		head = env_list->next;
		free_env_one(env_list);
		env_list = head;
	}
}
//===========================env_list===========================

int	check_pre_word(int pre_type)
{
	if (pre_type == token_env2)
		return (0);
	else if (pre_type == token_word)
		return (0);
	else if (pre_type == quotes)
		return (0);
	else if (pre_type == env_word)
		return (0);
	return (1);
}

int	check_next_word(t_tk **lex_list)
{
	if (!*lex_list)
		return (1);
	if ((*lex_list)->next->w_type == space)
		(*lex_list) = (*lex_list)->next;
	(*lex_list) = (*lex_list)->next;
	if ((*lex_list)->w_type == token_env2)
		return (0);
	else if ((*lex_list)->w_type == token_word)
		return (0);
	else if ((*lex_list)->w_type == quotes)
		return (0);
	else if ((*lex_list)->w_type == env_word)
		return (0);
	return (1);
}

t_redir	*moveto_redirs_end(t_redir *head)
{
	while (head->next)
		head = head->next;
	return (head);
}

t_cmd	*moveto_argvs_end(t_cmd *head)
{
	while (head->next)
		head = head->next;
	return (head);
}

int	fill_arg(t_cmd *arg_v, t_tk *lex_list)
{
	t_cmd	*tail;
	char	*join_str;

	tail = moveto_argvs_end(arg_v);
	if (tail->cmd)
		join_str = ft_strjoin(tail->cmd, lex_list->w_content);
	else
		join_str = ft_strdup(lex_list->w_content);
	if (!join_str)
		return (1);
	free(tail->cmd);
	tail->cmd = join_str;
	if (tail->w_type == token_word || tail->w_type == none)
		tail->w_type = lex_list->w_type;
	return (0);
}

int	type_word(t_tk **lex_list, t_par *par_list)
{
	printf("\n=word= : =%s=\n", (*lex_list)->w_content);//
	if (fill_arg(par_list->argvs , *lex_list))
		return (1);
	par_list->previous_type = (*lex_list)->w_type;
	(*lex_list) = (*lex_list)->next;
	return (0);
}

int	type_eoc(t_tk **lex_list, t_par *par_list)
{
	t_cmd	*end;

	end = moveto_argvs_end(par_list->argvs);
	init_one_argv(&end->next);
	end->next->cmd = ft_strdup("EOC");
	end->next->w_type = token_eoc;
	*lex_list = (*lex_list)->next;
	return (0);
}

int	type_pipe(t_tk **lex_list, t_par **par_list)
{
	t_cmd	*end;

	if (check_pre_word((*par_list)->previous_type))
		return (make_func_err(e_pipe));
	if (check_next_word(lex_list))
		return (make_func_err(e_pipe));
	end = moveto_argvs_end((*par_list)->argvs);
	init_one_argv(&end->next);
	end->next->cmd = ft_strdup("EOC");
	end->next->w_type = token_eoc;
	if ((*lex_list)->w_type == quotes)
		(*lex_list) = (*lex_list)->next;
	if (init_par_list(&(*par_list)->next))
		return (1);
	*par_list = (*par_list)->next;
	if (fill_arg((*par_list)->argvs, *lex_list))
		return (1);
	(*par_list)->previous_type = (*lex_list)->w_type;
	(*lex_list) = (*lex_list)->next;
	return (0);
}

int	type_env(t_tk **lex_list, t_par *par_list)
{
	if (fill_arg(par_list->argvs , *lex_list))
		return (1);
	par_list->previous_type = env_word;
	(*lex_list) = (*lex_list)->next;
	return (0);
}

int	type_env2(t_tk **lex_list, t_par *par_list)
{
	if (fill_arg(par_list->argvs , *lex_list))
		return (1);
	par_list->previous_type = env_word;
	(*lex_list) = (*lex_list)->next;
	return (0);
}

/*
int	check_next_quos(t_tk **lex_list)
{
	if ((*lex_list)->w_type == quot_word)
		return (1);
	else if ((*lex_list)->w_type == token_env)
		return (1);
	else if ((*lex_list)->w_type == token_env2)
		return (1);
	return (0);
}
int	type_qword(t_tk **lex_list, t_par *par_list)
{
	printf("quoword : =%s=\n", (*lex_list)->w_content);//
	par_list->previous_type = (*lex_list)->w_type;
	(*lex_list) = (*lex_list)->next;
	return (0);
}
*/
int	type_quotes(t_tk **lex_list, t_par *par_list)
{
	printf("quoword : =%s=\n", (*lex_list)->w_content);//
	if (fill_arg(par_list->argvs , (*lex_list)))
		return (1);
	par_list->previous_type = (*lex_list)->w_type;
	printf("bbbquoword : =%s=\n", (*lex_list)->w_content);//
	(*lex_list) = (*lex_list)->next;
	printf("nnnquoword : =%s=\n", (*lex_list)->w_content);//
/*	while (check_next_quos(lex_list))
	{
		if ((*lex_list)->w_type == quot_word)
			type_qword(lex_list, par_list);
		else if ((*lex_list)->w_type == token_env)
			type_env(lex_list, par_list);
		else
			type_env2(lex_list, par_list);
	}*/
	return (0);
}

int	redir_word(t_tk **lex_list)
{
	if (!*lex_list)
		return (0);
	if ((*lex_list)->w_type == space)
		return (0);
	if ((*lex_list)->w_type == token_env2)
		return (1);
	else if ((*lex_list)->w_type == token_word)
		return (1);
	else if ((*lex_list)->w_type == quotes)
		return (1);
	else if ((*lex_list)->w_type == env_word)
		return (1);
	return (0);
}

int	fill_redir(t_par *par_list, t_tk **lex_list, int w_type)
{
	t_redir	*tail;
	char		*join_str;

	tail = moveto_redirs_end(par_list->redirs);
	tail->r_type = w_type;
	while (redir_word(lex_list))
	{
		if (tail->r_word)
		{
			join_str = ft_strjoin(tail->r_word, (*lex_list)->w_content);
			free(tail->r_word);
		}
		else
			join_str = ft_strdup((*lex_list)->w_content);
		if (!join_str)
			return (1);
		tail->r_word = join_str;
		par_list->previous_type = (*lex_list)->w_type;
		(*lex_list) = (*lex_list)->next;
	}
	init_one_redir(&tail->next);
	return (0);
}



int	type_inre(t_tk **lex_list, t_par *par_list)
{
	if (check_pre_word(par_list->previous_type))
		return (make_func_err(e_inrd));
	if (check_next_word(lex_list))
		return (make_func_err(e_inrd));
	printf("\n=std_in_rd=\n");//
	if (fill_redir(par_list, lex_list, token_inrd))
			return (1);
	
	return (0);
}

int	type_outrd(t_tk **lex_list, t_par *par_list)
{
	if (check_pre_word(par_list->previous_type))
		return (make_func_err(e_outrd));
	if (check_next_word(lex_list))
		return (make_func_err(e_outrd));
	printf("\n=out_in_rd=\n");//
	if (fill_redir(par_list, lex_list, token_outrd))
		return (1);
	return (0);
}

int	type_aprd(t_tk **lex_list, t_par *par_list)
{
	if (check_pre_word(par_list->previous_type))
		return (make_func_err(e_aprd));
	if (check_next_word(lex_list))
		return (make_func_err(e_aprd));
	printf("\n=ap_in_rd=\n");//
	if (fill_redir(par_list, lex_list, token_aprd))
		return (1);
	return (0);
}

int	type_heredoc(t_tk **lex_list, t_par *par_list)
{
	if (check_pre_word(par_list->previous_type))
		return (make_func_err(e_heredoc));
	if (check_next_word(lex_list))
		return (make_func_err(e_heredoc));
	printf("\n=heardoc_rd=\n");//
	if (fill_redir(par_list, lex_list, token_heredoc))
		return (1);
	return (0);
}

int	type_space(t_tk **lex_list, t_par *par_list)
{
	t_cmd	*end;

	printf("\nspace: = =\n");//
	if (!check_next_word(lex_list))
	{
		end = moveto_argvs_end(par_list->argvs);
		init_one_argv(&end->next);
	}
	return (0);
}

int	todo(t_tk **lex)//
{
	printf("\nto do type: =%s=\n", e_char[(*lex)->w_type]);//
	(*lex) = (*lex)->next;
	return (0);
}
int	pasers_type_check(t_tk **lex, t_par **par_list)
{
	t_tk	*lex_head;

	lex_head = *lex;
	if (lex_head->w_type == token_word)
		return (type_word(lex, *par_list));
	else if (lex_head->w_type == token_pipe)
		return (type_pipe(lex, par_list));
	else if (lex_head->w_type == env_word)
		return (type_env(lex, *par_list));
	else if (lex_head->w_type == token_env2)
		return (type_env2(lex, *par_list));
	else if (lex_head->w_type == quotes)
		return (type_quotes(lex, *par_list));
	else if (lex_head->w_type == token_inrd)
		return (type_inre(lex, *par_list));
	else if (lex_head->w_type == token_outrd)
		return (type_outrd(lex, *par_list));
	else if (lex_head->w_type == token_aprd)
		return (type_aprd(lex, *par_list));
	else if (lex_head->w_type == token_heredoc)
		return (type_heredoc(lex, *par_list));
	else if (lex_head->w_type == space)
		return (type_space(lex, *par_list));
	else if (lex_head->w_type == token_eoc)
		return (type_eoc(lex, *par_list));
	else
		return (todo(lex));
}

int	check_syntax(t_tk *lex_list, t_par *par_list)
{
	if (lex_list->w_type == space)
		lex_list = lex_list->next;
	while (lex_list)
	{
		if (pasers_type_check(&lex_list, &par_list))
			return (print_argv_redirs(par_list), 1);
	}
	return (print_argv_redirs(par_list), 0);
}

int	do_here_doc(t_redir *redirs)
{
	int	pipe_fd[2];
	char	*heredoc_line;

	if (pipe(pipe_fd) == -1)
		return (perror("do_here_doc, pipe"), 1);
	printf("end_word: %s \n", redirs->r_word);
	while (1)
	{
		heredoc_line = readline(">");
		if (!heredoc_line)
		{
			close(pipe_fd[0]);
			close(pipe_fd[1]);
			return (perror("h_readline"), 1);
		}
		if (ft_strncmp(heredoc_line, redirs->r_word, ft_strlen(redirs->r_word) + 1) == 0)
		{
			free(heredoc_line);
			close(pipe_fd[1]);
			redirs->heredoc_fd = pipe_fd[0];
			return (0);
		}
		ft_putstr_fd(heredoc_line, pipe_fd[1]);
		ft_putstr_fd("\n", pipe_fd[1]);
	}
	close(pipe_fd[0]);
	close(pipe_fd[1]);
	return (0);
}

void	print_here_doc_pipe(t_redir *redirs)
{
	char buf[TMPBUF];
	int	read_len;

	while (redirs)
	{
		if (redirs->r_type == token_heredoc)
		{
			printf("\n\n====heredoc: %d start====\n\n", redirs->heredoc_fd);
			while ((read_len = read(redirs->heredoc_fd, buf, TMPBUF)) > 0)
				write(1, buf, read_len);
			printf("\n\n=======heredoc  end=======\n\n");
		}
		redirs = redirs->next;
	}
}


int	check_here_doc(t_redir *redirs)
{
	
	while(redirs)
	{
		if (redirs->r_type == token_heredoc)
		{
			if (do_here_doc(redirs))
				return (1);
			write(1, "\n", 1);
		}
		redirs = redirs->next;
	}
	return (0);
}


int	parser(t_bundle *bundle)
{
	t_tk	*lex_list;

	lex_list = bundle->lex_list;
	if (init_par_list(&bundle->par_list))
		return (free_all_par(&bundle->par_list), 1);
	if (check_syntax(lex_list, bundle->par_list))// env 테스트용
	{
		printf("\n====!syntax error!====\n");
		return (free_all_par(&bundle->par_list), 1);
	}
	if (check_here_doc(bundle->par_list->redirs))
	{
		printf("\nfail_here_doc\n");
		return (1);
	}
	printf("\n====!end paser!====\n");
	print_par_list(bundle->par_list);
	print_here_doc_pipe(bundle->par_list->redirs);
	return (0);
}

//------------------------paser
//-------------------------------executer

char	*check_env_word(char *envstr, int *i)
{
	int	j;
	char	*env_name;

	j = *i + 1;
	while (envstr[j])
	{
		if (!ft_isalpha(envstr[j]) && !(envstr[j] == '_') && !ft_isdigit(envstr[j]))
				break ;
		j++;
	}
	env_name = ft_strndup(&envstr[*i + 1], j - 1 - *i);
	if (!env_name)
		return (NULL);
	*i = j - 1;
	return (env_name);
}

int	change_to_content(t_env *env_list, char **env_name)
{
	t_env *find_env;
	char	*content;

	find_env = find_env_name(env_list, *env_name);
	if (!find_env)
		content = NULL;
	else
	{
		content = ft_strdup(find_env->env_content);
		if (!content)
			return (1);
	}
	free(*env_name);
	*env_name = content;
	return (0);
}

//===============================================================================================

char	*wrap_strjoin(char *str1, const char *str2)
{
	char	*newstr;

	/* ── ① 빠른 경로 : NULL 조합 ─────────────────────────────────────── */
	if (!str1 && !str2)
		return (ft_strdup(""));
	if (!str1)
		return (ft_strdup(str2 ? str2 : ""));
	if (!str2)
		return (ft_strdup(str1));

	/* ── ② 일반 경로 : 두 문자열 모두 존재 ─────────────────────────── */
	newstr = ft_strjoin(str1, str2);          /* <-- str1 내용 복사 */
	if (!newstr)                              /* malloc 실패 → str1 회수 */
	{
		free(str1);
		return (NULL);
	}
	free(str1);                               /* ★ 성공해도 원본 str1 해제 */
	return (newstr);                          /* 호출자가 ‘새 버퍼’만 소유 */
}

int ft_isalnum(int c)
{
	return ((c >= '0' && c <= '9')
		|| (c >= 'A' && c <= 'Z')
		|| (c >= 'a' && c <= 'z'));
}

static int	num_len(int n)
{
	int	len = (n <= 0);

	while (n)
	{
		n /= 10;
		++len;
	}
	return (len);
}

char	*ft_itoa(int n)
{
	char	*str;
	long	nbr;
	int		len;

	nbr = n;
	len = num_len(nbr);
	str = (char *)malloc(len + 1);
	if (!str)
		return (NULL);
	str[len] = '\0';
	if (nbr == 0)
		str[0] = '0';
	if (nbr < 0)
	{
		str[0] = '-';
		nbr = -nbr;
	}
	while (nbr)
	{
		str[--len] = (nbr % 10) + '0';
		nbr /= 10;
	}
	return (str);
}

static int	append_chr(char **dst, char c)
{
	char	buff[2];

	buff[0] = c;
	buff[1] = '\0';
	*dst = wrap_strjoin(*dst, buff);
	if (!*dst)
		return (perror("malloc(append_chr)"), 1);
	return (0);
}

/*
** $ 토큰 처리
**  p   : '$' 바로 다음 문자를 가리킨다
**  dst : 누적 결과 문자열
**  env : 환경변수 연결리스트
*/

static const char	*handle_dollar(const char *p, char **dst, t_env *env, int last_status)
{
	char	*var_name;
	char	*var_val;

	if (*p == '?')							/* $? → 마지막 종료값 */
	{
		var_val = ft_itoa(last_status);
		if (!var_val)
			return (perror("malloc(ft_itoa)"), (char *)NULL);
		*dst = wrap_strjoin(*dst, var_val);
		free(var_val);
		if (!*dst)
			return (perror("malloc(handle_dollar)"), (char *)NULL);
		return (++p);
	}
	if (!ft_isalpha(*p) && *p != '_')		/* 유효하지 않은 식별자 */
	{
		if (append_chr(dst, '$'))
			return (char *)NULL;
		return (p);
	}
	/* 변수 이름 파싱 */
	const char *start = p;
	while (ft_isalnum(*p) || *p == '_')
		++p;
	var_name = ft_strndup(start, p - start);
	if (!var_name)
		return (perror("malloc(ft_strndup)"), NULL);
	/* 값 가져오기 (정의 안 됐으면 NULL → 빈 문자열) */
	if (change_to_content(env, &var_name))
		return (free(var_name), NULL);
	*dst = wrap_strjoin(*dst, var_name ? var_name : "");
	free(var_name);
	if (!*dst)
		return (perror("malloc(handle_dollar)"), NULL);
	return (p);
}




char	*expand_all(const char *src, t_env *env_list, int last_status)
{
	int				in_squote;
	int				in_dquote;
	const char		*p;
	char			*out;

	if (!src)
		return (NULL);
	in_squote = 0;
	in_dquote = 0;
	p = src;
	out = ft_strdup("");
	if (!out)
		return (perror("malloc(ft_strdup)"), NULL);
	while (*p)
	{
		if (*p == '\'' && !in_dquote)				/* 싱글쿼트 토글 */
			in_squote = !in_squote, ++p;
		else if (*p == '"' && !in_squote)			/* 더블쿼트 토글 */
			in_dquote = !in_dquote, ++p;
		else if (*p == '$' && !in_squote)			/* $ 확장 (dq/outside) */
		{
			p = handle_dollar(p + 1, &out, env_list, last_status);
			if (!p)									/* malloc error */
				return (NULL);
		}
		else										/* 일반 문자 */
		{
			if (append_chr(&out, *p))
				return (NULL);
			++p;
		}
	}
	return (out);
}

int	expand_cmd(t_cmd *node, t_env *env_list)
{
	char	*expanded;

	if (!node || !node->cmd)
		return (1);
	expanded = expand_all(node->cmd, env_list, g_sig_int);
	if (!expanded)
		return (1);
	free(node->cmd);
	node->cmd = expanded;
	return (0);
}

int	expand_redir(t_redir *node, t_env *env_list)
{
	char	*expanded;

	if (!node || !node->r_word)
		return (1);
	expanded = expand_all(node->r_word, env_list, g_sig_int);
	if (!expanded)
		return (1);
	free(node->r_word);
	node->r_word = expanded;
	return (0);
}


//===============================================================================================
/*
int	need_to_change(t_cmd *argvs, t_env *env_list, int *i)
{
	//int	meta_ind;
	(void) env_list;
	char	*join_word;
	char	*pre_word;
	char	*env_word;
	char	*next_word;
	int	join_len;

	//meta_ind = find_meta(argvs->cmd, '$');
	
	
	pre_word = ft_strndup(argvs->cmd, *i);
	if (!pre_word)
		return (1);
	printf("preword: =%s=\n", pre_word);
	
	
	env_word = check_env_word(argvs->cmd, i);
	if (!env_word)
		return (free(pre_word), 1);
	if (change_to_content(env_list, &env_word))
		return (free(pre_word), 1);
	printf("env_word: =%s=\n", env_word ? env_word : "NULL");
	
	
	next_word = ft_strdup(&(argvs->cmd[*i + 1]));
	if (!next_word)
		return (free(pre_word), 1);
	printf("next_word: =%s=\n", next_word);
	
	
	join_word = wrap_strjoin(pre_word, env_word);
	free(pre_word);
	if (!join_word)
		return (free(next_word), 1);
	join_len = ft_strlen(join_word);


	pre_word = wrap_strjoin(join_word, next_word);
	free(join_word);
	free(next_word);
	free(env_word);
	if (!pre_word)
		return (1);
	free(argvs->cmd);
	argvs->cmd = pre_word;
	*i = join_len - 1;
	return (0);
}

int	remove_quot(t_cmd *argvs, t_env *env_list)
{
	int	i;

	i = 0;
	while ((argvs->cmd)[i])
	{
		if ((argvs->cmd)[i] == '\"')
		{
			ft_memmove(&(argvs->cmd)[i], &(argvs->cmd)[i + 1], ft_strlen(&(argvs->cmd)[i + 1]) + 1);
			while ((argvs->cmd)[i] != '\"')
			{
				if ((argvs->cmd)[i] == '$')
				{
					if (ft_isalpha((argvs->cmd)[i + 1]) || (argvs->cmd)[i + 1] == '_')
						if (need_to_change(argvs, env_list, &i))
							return (1);
					continue ;
				}
				i++;
			}
			ft_memmove(&(argvs->cmd)[i], &(argvs->cmd)[i + 1], ft_strlen(&(argvs->cmd)[i + 1]) + 1);
		}
		else if ((argvs->cmd)[i] == '$')
		{
			if (ft_isalpha((argvs->cmd)[i + 1]) || (argvs->cmd)[i + 1] == '_')
				if (need_to_change(argvs, env_list, &i))
					return (1);
			continue ;
		}
		else if ((argvs->cmd)[i] == '\'')
		{
			ft_memmove(&(argvs->cmd)[i], &(argvs->cmd)[i + 1], ft_strlen(&(argvs->cmd)[i + 1]) + 1);
			while ((argvs->cmd)[i] != '\'')
				i++;
			ft_memmove(&(argvs->cmd)[i], &(argvs->cmd)[i + 1], ft_strlen(&(argvs->cmd)[i + 1]) + 1);
			continue ;
		}
		i++;
	}
	return (0);
}
*/

int	expand_env(t_env *env_list, t_cmd *argvs, t_redir *redirs)
{
	while (argvs)
	{
		if (expand_cmd(argvs, env_list))
			return (1);
		argvs = argvs->next;
	}
	while (redirs)
	{
		if (redirs->r_type == token_heredoc || redirs->r_type == none)
		{
			redirs = redirs->next;
			continue ;
		}
		if (expand_redir(redirs, env_list))
			return (1);
		redirs = redirs->next;
	}
	return (0);
	
}
/* 내가 생각한것과 좀 다르게 작동 그냥 명령어 복붙 하면될 줄알았지만 이 방식의 경우 중간에 띄어쓰기 구분을 못함
즉 "a b" << 최종인자 2개임 현재 저렇게 들어오면 "a b" 하나의 인자 
그러다보니 전부다 버퍼에 다담은 후에(strjoin) split 진행하면될듯 싶음.
char *k = strjoin(현재 cmd)// 다음으로 넘어갈때 공백추가 > 반복
널도착 > split 진행 
   
int	make_argv_envs(t_cmd *argvs, t_env *env_list)
{
	
	while (argvs)

}
*/
int	executer(t_bundle *bundle)
{
	//printf("===========before\n");
	//print_argv_redirs(bundle->par_list);
	if (expand_env(bundle->env_list, bundle->par_list->argvs, bundle->par_list->redirs))
		return (1);
	//if (make_argv_envs(bundle->par_list->argvs, bundle->env_list))
		//return (1);
	//printf("===========after\n");

	print_argv_redirs(bundle->par_list);
	return (0);
}
//-------------------------------executer
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

void	free_bundle(t_bundle *tmp, int end)
{
	if (tmp->read_line)
		free(tmp->read_line);
	if (tmp->lex_list)
		free_all_lex(&tmp->lex_list);
	if (tmp->par_list)
		free_all_par(&tmp->par_list);
	tmp->read_line = NULL;
	if (end)
	{
		if (tmp->env_list)
			free_env(tmp->env_list);
		free(tmp);
		tmp = NULL;
	}
}

int	set_bundle(t_bundle **tmp)
{
	(*tmp) = malloc(sizeof(t_bundle));
	if (!*tmp)
		return (perror("malloc(set_bundle)"), 1);
	(*tmp)->env_list = NULL;
	(*tmp)->read_line = NULL;
	(*tmp)->lex_list = NULL;
	(*tmp)->par_list = NULL;
	return (0);
}

int	init_envlist(char **envp, t_env **envlist)
{
	int	i;

	i = 0;
	while (envp[i])
	{
		if (tmp_add_env_list(envlist, envp[i]))
			return (1);
		i++;
	}
	return (0);
}

int	main(int argc, char **argv, char **envp)
{
	(void)argc, (void)argv;
	t_bundle	*tmp;

	if (signal_set() || set_bundle(&tmp) || init_envlist(envp, &tmp->env_list))
		return (1);
	print_env(tmp->env_list);
	while (1)
	{
		tmp->read_line = readline("readline>");
		if (!tmp->read_line)
		{
			if (errno == 0) //ctrl+D
				return (free_bundle(tmp, 1), write(1, "exit\n", 5), 0);
			perror("readline");
			return (free_bundle(tmp, 1), 1);
		}
		else
		{
			if (*tmp->read_line)
			{
				if (lexer(tmp))
				{
					add_history(tmp->read_line);
					free_bundle(tmp, 0);
					continue ;
				}
				print_lex(tmp->lex_list, tmp->read_line);
				add_history(tmp->read_line);
				if (parser(tmp)) 
				{
					free_bundle(tmp, 0);
					continue ;
				}
				if (executer(tmp))
				{
					free_bundle(tmp, 0);
					continue ;
				}
			}
		}
		free_bundle(tmp, 0);
	}
}
