NAME = libftprintf.a
LNAME = libft.a
CC = cc
CFLAGS = -Wall -Wextra -Werror -I ./libft

SRCS = ft_printf.c \
       ft_pbase.c \
       ft_pputnbr.c \
       ft_print_chr.c \
       ft_print_str.c \
       ft_print_ptr.c \
       ft_print_dec.c \
       ft_print_int.c \
       ft_print_uit.c \
       ft_print_hex.c \
       ft_print_lex.c \
       ft_print_check.c

OBJS = $(SRCS:.c=.o)

LIBFT_DIR = libft
LIBFT = $(LIBFT_DIR)/$(LNAME)

all: $(NAME)

$(NAME): $(OBJS) $(LIBFT)
	cp $(LIBFT) $(NAME)
	ar rcs $(NAME) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIBFT):
	make -C $(LIBFT_DIR)

clean:
	rm -f $(OBJS)
	make -C $(LIBFT_DIR) clean

fclean: clean
	rm -f $(NAME)
	make -C $(LIBFT_DIR) fclean

re: fclean all

.PHONY: all clean fclean re
