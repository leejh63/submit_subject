#ifndef MINISHELL_H
# define MINISHELL_H

# include <readline/readline.h>
# include <readline/history.h>
# include <stdlib.h>
# include <stdio.h>
# include <unistd.h>
# include <fcntl.h>
# include <signal.h>
# include <errno.h>
# include <sys/wait.h>
# include "libft/ft_printf.h"

typedef enum e_tk_type
{
	ws,
	word,
	pip,
	inrd,
	outrd,
	aprd,
	heredoc,
	env,
	sq,
	dq,
	end,
	error	//temporary 없앨수도?
}	t_tt;

typedef	struct	s_tok
{
	t_tt	type;
	char	*str;
	struct	s_tok	*next;
	int		hd_fd;
}	t_tok;

typedef	struct	s_redir
{
	t_tt	type;
	t_tok	*str;
	char	**sstr;
	struct	s_redir	*next;
	int		hd_fd;
}	t_redir;

typedef struct	s_word
{
	char	*str;
	struct	s_word	*next;
}	t_word;

typedef struct s_exp
{
	t_redir	*redir;
	t_tok	*cmd;
	char	**args;
	struct s_exp	*next;
}	t_exp;

typedef	struct s_env
{
	struct s_env	*next;
	char	*key;
	char	*val;
}	t_env;

int scan_and_parse(char *line, t_exp **chunks, t_env *envs);

t_tok	*lex(char *line);
void	get_one_chunk(t_tok **tok, t_tok **chunk);
t_tok	*prev_tok(t_tok *head, t_tok *this);
t_exp	*construct(t_tok *tokens, t_env *envs);
int 	validate_syntax(t_tok *toks);
//t_tok	*ftok_lstnew(char *str, t_tt type);
void	del_tok(t_tok *tok);
void	ftok_lstadd_back(t_tok **lst, t_tok *new);
void	ftok_lstclear(t_tok **lst);
t_tok	*ftok_lstlast(t_tok *lst);
void	fexp_lstadd_back(t_exp **lst, t_exp *new);
void	print_exps(t_exp *exp);
void	ft_redir_lstadd_back(t_redir **lst, t_redir *new);
char **unquote_and_expand_param(t_tok *toks, t_env *envs);

char **word_to_str(t_word *words);
int	word_lstsize(t_word *words);
int	word_append(t_word *cur, char *str);
void	word_next(t_word **words, t_word **cur);
int	do_heredoc(char *limiter);
int	handle_heredocs(t_tok *token);
char *get_delim(t_tok **tok);


void	print_toks(t_tok *tcur);

int is_general_char(char c, int in_quote);
int	is_whitespace(char c);
char *ft_strndup(const char *str, size_t n);
int	ft_strcmp(const char *s1, const char *s2);

void	free_exp(t_exp *exp);
void	free_redir(t_redir *redir);
void free_double_ptr(char **ptr);
void	close_all_hd_fd(t_tok *token);

//==

int	check_fd_redirect(t_exp *exp, int in_pipe, int out_pipe);
int	redir_all(t_redir *redirs);
int	wrap_open(int flags, char *path, int (*f_redir)(int));
int	redir_out(int redir_fd);
int	redir_in(int redir_fd);

int	do_built_in(t_exp *exp, t_env **envs, int out_fd);
int init_bash(char **envp, t_env **lst);
int	set_env(t_env *env, char *key, char *val);
char	*get_env(t_env *envs, char *key);
void	ft_env_add_back(t_env **lst, t_env *new_node);
t_env	*make_env(char *key, char *val);
void	free_env(t_env *env);
int	check_find_path(char **argv, t_env *env);
int	check_built_in(char *argv);
int exec(t_exp *toks, t_env *envs);
//--------print
void print_argv(char **argv);
void print_env_list(t_env *head);
void	free_exp_envs(t_exp *exp, t_env *envs);
int	in_func_exit(t_exp *exp, t_env *envs, int in_fd, int out_fd);
//==
int	get_q(t_env *lst);
void	set_q(t_env *lst, int i);

void	set_signal_handler(void);
void	set_signum(int sig_no);
void	sig_int_func(int sig_no);
extern volatile sig_atomic_t	g_signum;
void	ft_perror(char *s1, char *s2, char *s3, int	strerr);
# endif
