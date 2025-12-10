/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/15 17:53:51 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/15 17:53:53 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
	error
}	t_tt;

typedef struct s_tok
{
	t_tt			type;
	char			*str;
	struct s_tok	*next;
	int				hd_fd;
}	t_tok;

typedef struct s_redir
{
	t_tt			type;
	t_tok			*str;
	char			**sstr;
	struct s_redir	*next;
	int				hd_fd;
}	t_redir;

typedef struct s_word
{
	char			*str;
	struct s_word	*next;
}	t_word;

typedef struct s_exp
{
	t_redir			*redir;
	t_tok			*cmd;
	char			**args;
	struct s_exp	*next;
}	t_exp;

typedef struct s_env
{
	struct s_env	*next;
	char			*key;
	char			*val;
}	t_env;

int		handle_heredocs(t_tok *head);
void	close_all_hd_fd(t_tok *token);

int		validate_syntax(t_tok *toks);
t_tok	*next_tok(t_tok *tok, t_tt type);
t_tok	*lex(char *line);

t_exp	*construct(t_tok *tokens, t_env *envs);
char	**unquote_and_expand_param(t_tok *toks, t_env *envs);
void	get_one_chunk(t_tok **tok, t_tok **chunk);
t_tok	*get_wstok(void);
t_tok	*prev_tok(t_tok *head, t_tok *this);

int		check_fd_redirect(t_exp *exp, int in_pipe, int out_pipe);
int		redir_all(t_redir *redirs, int flags);
int		wrap_open(int flags, char *path, int (*f_redir)(int));
int		redir_out(int redir_fd);
int		redir_in(int redir_fd);

int		do_built_in(t_exp *exp, t_env **envs, int out_fd);
int		check_built_in(char *args);
int		argv_len(char **argv);

int		envs_len(t_env *lst);
char	**env_to_array(t_env *lst);
int		exec(t_exp *toks, t_env *envs);

void	fexp_lstadd_back(t_exp **lst, t_exp *new);
t_exp	*fexp_lstlast(t_exp *lst);

int		check_find_path(char **argv, t_env *env);
int		find_path_in_env(t_env *env, char **argv);
int		join_path(char *colon, char **access_path, char *argv);
int		get_spilt_colon(t_env *env, char ***colon);
void	free_split(char **split);

void	child_doing(t_exp *cur, t_env *envs, int pipes[3][2]);
int		wait_pid_end(int pid, t_env *envs);
void	built_exit_fork(t_exp *cur, t_env *envs, int pipes[3][2]);
void	fork_error(int pipes[3][2], t_exp *cur);

void	free_exp_envs(t_exp *exp, t_env *envs);
void	free_exp(t_exp *exp);
void	free_env(t_env *env);
void	free_redir(t_redir *redir);
void	free_double_ptr(char **ptr);

int		in_func_export(char **argv, t_env **envs, int out_fd);
int		export_env_val(char *argv, char **val, int *equal);
void	write_export(t_env *envs, int out_fd);

int		in_func_cd(char **args, t_env *envs);

int		in_func_echo(char **argv, t_env *envs, int out_fd);
void	echo_write(char **argv, int out_fd, int argc);

int		in_func_env(char **argv, t_env *envs, int out_fd);
int		find_envlist_key(t_env *envs, char *key, char *val);
int		check_env_name(char *key, int *ex_sig);

int		in_func_exit(t_exp *exp, t_env *envs, int in_fd, int out_fd);
int		tmp_atoi(const char *nptr);
int		in_range(const char *nptr);
int		is_numeric(const char *nptr);

int		in_func_pwd(char **argv, t_env *envs, int out_fd);

int		in_func_unset(char **argv, t_env **envs, int out_fd);
void	unset_one_env(t_env *head, t_env *pre, t_env **envs);

int		init_bash(char **envp, t_env **lst);
int		set_pwd(t_env **lst);
int		make_t_env(char **envp, t_env **lst);
int		set_t_env(char *envp, char **val, char **key);
int		make_first_env(t_env **lst);

char	*get_env(t_env *envs, char *key);
char	*take_env_key(int equal, char *envp);
int		find_equal(char *env_str, int	*equal);
int		set_env(t_env *env, char *key, char *val);
t_env	*make_env(char *key, char *val);

int		get_q(t_env *lst);
void	set_q(t_env *lst, int i);
void	ft_env_add_back(t_env **lst, t_env *new_node);

void	pipe_error(int pipes[3][2], t_exp *cur);
int		pipe_setting(t_exp *cur, int pipes[3][2]);
void	make_pipe(int pipes[3][2]);

void	print_exps(t_exp *exp);
void	print_redir(t_redir *tcur);
void	print_dptr(char **str);

int		pipe_and_fork(t_exp *toks, t_env *envs);
void	next_cur(t_exp **cur);

void	ft_redir_lstadd_back(t_redir **lst, t_redir *new);

int		redir_exe_builtin(t_exp *exp, t_env *envs);
void	set_save_fd(int save_fd[2]);
int		reset_fd(int *save_fd);

void	scan_and_parse(char *line, t_exp **chunks, t_env *envs);
int		parse_status(t_env *envs, t_tok *toks);
void	hd_free_and_exit(t_tok *toks, t_env *envs, int status);
int		too_many_heredoc(t_tok	*toks);
int		is_all_ws(char *str);

int		set_shl_lv(t_env **lst);
int		lv_up_shl(t_env **lst, char *val);
int		make_shl(t_env **lst);

void	set_signal_handler(void);
void	set_signum(int sig_no);
void	sig_int_func(int sig_no);
int		get_signum(void);
void	heredoc_sigint(int sig_no);

t_tok	*ftok_lstlast(t_tok *lst);
void	ftok_lstclear(t_tok **lst);
void	del_tok(t_tok *tok);
void	ftok_lstadd_back(t_tok **lst, t_tok *new);

void	ft_perror(char *s1, char *s2, char *s3, int strerr);
int		ft_strcmp(const char *s1, const char *s2);
int		is_general_char(char c, int in_quote);
int		is_whitespace(char c);
char	*ft_strndup(const char *str, size_t n);

void	word_next(t_word **words, t_word **cur);
int		word_append(t_word *cur, char *str);
int		word_lstsize(t_word *words);
char	**word_to_str(t_word *words);

t_tok	*handle_whitespace(char **line);
t_tok	*handle_end(void);
t_tok	*handle_word(char **line, int in_quote);
t_tok	*handle_pipe(char **line);
t_tok	*handle_quote(char **line, int *in_quote);
t_tok	*handle_redirect(char **line);
t_tok	*handle_dollar(char **line);

int		construct_parts(t_tok **cur, t_exp **head, t_exp **node, t_env *envs);

int		null_func(void);

#endif
