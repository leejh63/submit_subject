# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: kkeum <kkeum@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/04/04 16:10:05 by kkeum             #+#    #+#              #
#    Updated: 2025/05/04 19:50:10 by kkeum            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS =	ft_ntoa.c \
		ft_parse_fmt.c \
		ft_printf.c \
		ft_putc.c \
		ft_putd.c \
		ft_putp.c \
		ft_puts.c \
		ft_putu.c \
		ft_put_utils.c \
		ft_putx.c \
		ft_write_fmt.c \
		put_until_fmt.c \
		reverse_str.c \
		ft_utils.c \
		get_next_line.c

OBJS = $(SRCS:.c=.o)

LIBFT_DIR = ./libft

LIBFT = $(LIBFT_DIR)/libft.a

NAME = libftprintf.a

all: $(NAME)

$(NAME): $(OBJS) $(LIBFT) 
	cp $(LIBFT) $(NAME)
	ar rsc $@ $(OBJS)

%.o: %.c
	cc -Wall -Werror -Wextra -c $< -o $@

$(LIBFT):
	make bonus -C $(LIBFT_DIR)

bonus: $(NAME)

clean:
	rm -f $(OBJS) 
	make fclean -C $(LIBFT_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re bonus
