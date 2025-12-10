NAME            := push_swap

CC              := cc
CFLAGS          := -Wall -Wextra -Werror
INCLUDES        := -I.

SRCS            := \
	ft_strlcpy.c \
	ft_isdigit.c \
	ft_split.c \
	ft_strlen.c \
	ft_pbase.c \
	ft_pputnbr.c \
	ft_print_check.c \
	ft_print_chr.c \
	ft_print_dec.c \
	ft_print_hex.c \
	ft_print_int.c \
	ft_print_lex.c \
	ft_print_ptr.c \
	ft_print_str.c \
	ft_print_uit.c \
	ft_printf.c \
	swap_error.c \
	swap_free.c \
	swap_list.c \
	swap_pushf.c \
	swap_rotate.c \
	swap_rrotate.c \
	swap_swapf.c \
	check_table_a.c \
	fill_check_box.c \
	fill_sort_table.c \
	fill_stack_a.c \
	find_max_len.c \
	find_maxlen_init.c \
	find_set_stack.c \
	init_table_stack.c \
	move_b_init.c \
	sort_logic.c \
	sort_up_three_a.c \
	sorting_func.c \
	table_set.c \
	main.c

OBJS            := $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
