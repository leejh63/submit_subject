CC        = cc
CFLAGS    = -Wall -Wextra -Werror -I. -Iminilibx-linux

MLX_DIR   = minilibx-linux
MLX_LIB   = libmlx_Linux.a
MLX_FLAGS = -L$(MLX_DIR) -lmlx_Linux -lXext -lX11 -lm

NAME      = fractol

SRCS      = \
	err_num.c \
	ft_func.c \
	mlx_struct.c \
	num_struct.c \
	convert_complex.c \
	check_fractal.c \
	mandel_fractal.c \
	julia_fractal.c \
	dot_pixel.c \
	mouse.c \
	keyboard.c \
	event_handler.c \
	arg_check.c \
	mand_arg_check.c \
	julia_arg_check.c \
	main.c

OBJS      = $(SRCS:.c=.o)

all: $(MLX_DIR)/$(MLX_LIB) $(NAME)

$(MLX_DIR)/$(MLX_LIB):
	@$(MAKE) -C $(MLX_DIR)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(MLX_FLAGS) -o $(NAME)
	clear

%.o: %.c fractol.h
	$(CC) $(CFLAGS) -c $< -o $@
	clear

clean:
	rm -f $(OBJS)
	@$(MAKE) -C $(MLX_DIR) clean
	clear

fclean: clean
	rm -f $(NAME)
	@$(MAKE) -C $(MLX_DIR) clean
	clear

re: fclean all

.PHONY: all clean fclean re
