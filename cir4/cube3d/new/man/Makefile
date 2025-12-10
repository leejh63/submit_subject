# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: kkeum <kkeum@student.42gyeongsan.kr>       +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/27 15:04:17 by kkeum             #+#    #+#              #
#    Updated: 2025/05/28 20:01:56 by kkeum            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRCS =	callback.c \
		color.c \
		cub3d.c \
		dda_new.c \
		ft_exit.c \
		ft_utils.c \
		map_maker.c \
		moves.c \
		parse_map.c \
		validate_map.c \
		wall_checker.c \
		draw.c \


OBJS = $(SRCS:.c=.o)

LIBFT_DIR = ./libft

LIBFT = $(LIBFT_DIR)/libftprintf.a

MLX_DIR = minilibx-linux

MLX = $(MLX_DIR)/libmlx.a

NAME = cub3d

all: $(NAME)

$(NAME): $(OBJS) $(LIBFT) $(MLX)
	cc -Wall -Werror -Wextra $^ -o $@ $(MLX) -Iminilibx-linux -lX11 -lXext -lm -g

%.o: %.c
	cc -Wall -Werror -Wextra -c $< -o $@ -Iminilibx-linux -I$(LIBFT_DIR) -g

$(LIBFT):
	make bonus -C $(LIBFT_DIR)

$(MLX):
	make -C $(MLX_DIR)

bonus: $(NAME)

test: test.o $(LIBFT) $(MLX) ft_exit.o color.o ft_utils.o
	cc -Wall -Werror -Wextra $^ -o $@ $(LIBFT) $(MLX) -Iminilibx-linux -lX11 -lXext -lm -g

clean:
	rm -f $(OBJS)
	make fclean -C $(LIBFT_DIR)
	make clean -C $(MLX_DIR)
	rm -f test.o test

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re bonus
