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

#define TMPBUF 1024

typedef struct s_tk t_tk;

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
	char	*w_content;
	t_tk	*next;
	t_tk_type	w_type;
}	t_tk;

















