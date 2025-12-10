SRCS =	check_fd_redirect.c \
		construct_parts.c \
		do_built_in.c \
		exec.c \
		exp.c \
		find_path.c \
		fork_utils.c \
		frees.c \
		get_one_chunk.c \
		heredo.c \
		heredoc_signal.c \
		in_func_cd.c \
		in_func_echo.c \
		in_func_env.c \
		in_func_exit.c \
		in_func.export.c \
		in_func_pwd.c \
		in_func_unset.c \
		init_bash.c \
		init_bash_env.c \
		init_bash_utils.c \
		lex.c \
		parse.c \
		pipe_utils.c \
		print.c \
		reboot.c \
		redir.c \
		redir_exe_builtin.c \
		scan_and_parse.c \
		set_shl_lv.c \
		signal.c \
		tok.c \
		tok_handler2.c \
		tok_handler.c \
		unquote_and_expand.c \
		utils.c \
		validate_syntax.c \
		word.c
OBJS = $(SRCS:.c=.o)
NAME = minishell
LIBFT_DIR = libft
LIBFT = $(LIBFT_DIR)/libftprintf.a
SUPP = readline.supp

all: $(NAME)

$(NAME): $(OBJS) $(LIBFT)
	cc -Wall -Werror -Wextra -lreadline $^ -o $@ -g

%.o: %.c
	cc -Wall -Werror -Wextra -I$(LIBFT_DIR) -c $< -o $@ -g

$(LIBFT):
	make bonus -C $(LIBFT_DIR)

clean:
	rm -rf $(OBJS)
	make fclean -C $(LIBFT_DIR)

fclean:	clean
	rm -rf $(NAME) $(SUPP)

re:	fclean all

bonus:	all

.PHONY: all name clean fclean re
