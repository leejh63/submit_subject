SRCS = exp.c \
	heredo.c \
	lex.c	parse.c \
	reboot.c \
	redir.c \
	tok.c \
	utils.c \
	word.c \
	frees.c \
	print.c \
	do_bulit_in.c \
	find_path.c \
	check_fd_redirect.c \

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
	rm -rf $(NAME)

re:	fclean all

bonus:	all

sup: $(SUPP)
	echo "valgrind --suppressions=readline.supp --leak-check=full --show-leak-kinds=all --trace-childred=yes --track-fds=yes ./minishell"

$(SUPP):
	@printf "{\n\tleak readline\n\tMemcheck:Leak\n\t...\n\tfun:readline\n}\n{\n\tleak add_history\n\tMemcheck:Leak\n\t...\n\tfun:add_history\n}\n" >$(SUPP)

.PHONY: all name clean fclean re
