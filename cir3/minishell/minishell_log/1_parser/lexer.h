/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 14:34:51 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/07/02 14:34:52 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <readline/readline.h>	// readline 관련 헤더
#include <readline/history.h>	// history 관련 헤더
#include <unistd.h>		// write 관련 헤더
#include <stdio.h>		// printf, perror 관련 헤더
#include <signal.h>		// signal 관련 헤더
#include <errno.h>		// err_num 관련 헤더 
					//	perror의 경우 <errno.h> 헤더 포함이 표준
#include <stdlib.h>		// free, malloc 관련 헤더
#include <sys/ioctl.h>		// ioctl() prototypes, TIOCSTI 정의 관련 헤더

// --------------------------tmp------------------------
#define TMPBUF 1024

//   TMPBUF 사용
// init_lexc, init_par_list	함수
//s_lex buf, s_pstk tmp		변수
// 

//--------------------minishell.h---------------------------

typedef enum e_tk_type
{
	none,
	token_word,
	token_pipe,
	token_inrd,
	token_heredoc,
	token_outrd,
	token_aprd,
	token_env,
	token_env2,
	env_word,
	token_eof,
	token_eoc,
	quot_word,
	quotes,
	space,
}	t_tk_type;

typedef enum e_err
{
	success,
	e_init,
	e_malloc,
	e_squo,
	e_bquo,
	e_pipe,
	e_inrd,
	e_outrd,
	e_aprd,
	e_heredoc,
	e_env,
}	t_err;

typedef struct s_bundle
{
	char	*read_line;
	struct s_tk		*lex_list;
	struct s_par	*par_list; // 임시
	struct s_env	*env_list;
	char	**argv;
	char	**envs;
}	t_bundle;

//--------------------lexer.h--------------------------

typedef struct s_lex
{
	char	*rline;
	char	buf[TMPBUF];
	int	bind;
	int	ind;
	int	clen;
	int	stat;
}	t_lex;

typedef struct s_tk
{
	struct s_tk	*next;
	char		*w_content;
	t_tk_type	w_type;
}	t_tk;

//----------single_list.h

typedef struct s_slist
{
	struct s_slist	*next;
	void			*data;
}	t_slist;

//---------------parser.h---------------------------

typedef struct s_par
{
	struct s_par	*next;
	struct s_cmd	*argvs;
	struct s_redir	*redirs;
	int			previous_type;
	int			tmp_num;
}	t_par;


typedef struct s_redir
{
	struct s_redir	*next;
	char			*r_word;
	int			r_type;
	int			heredoc_fd;
}	t_redir;


typedef struct s_cmd
{
	struct s_cmd	*next;
	char			*cmd;
	int			w_type;
}	t_cmd;

//--------------------env_list.h
typedef struct s_env
{
	struct s_env	*prev;
	struct s_env	*next;
	char			*env_name;
	char			*env_content;
}	t_env;

























































