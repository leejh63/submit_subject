CC        = cc
CFLAGS    = -Wall -Wextra -Werror -I. -Iminilibx-linux

MLX_DIR   = minilibx-linux
MLX_LIB   = libmlx_Linux.a
MLX_FLAGS = -L$(MLX_DIR) -lmlx_Linux -lXext -lX11 -lm

NAME      = fractol

SRCS      = \
	ft_atoi.c \
	ft_atof.c \
	mlx_struct.c \
	num_struct.c \
	convert_complex.c \
	put_color.c \
	check_fractal.c \
	mandel_fractal.c \
	julia_fractal.c \
	tmain.c

OBJS      = $(SRCS:.c=.o)

all: $(MLX_DIR)/$(MLX_LIB) $(NAME)

$(MLX_DIR)/$(MLX_LIB):
	@$(MAKE) -C $(MLX_DIR)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(MLX_FLAGS) -o $(NAME)

%.o: %.c fractol.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)
	@$(MAKE) -C $(MLX_DIR) clean

fclean: clean
	rm -f $(NAME)
	@$(MAKE) -C $(MLX_DIR) clean

re: fclean all

.PHONY: all clean fclean re
