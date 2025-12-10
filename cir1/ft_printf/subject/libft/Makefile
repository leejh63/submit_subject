CC = cc
CFLAGS = -Wall -Wextra -Werror

SRC_D = .
OBJ_D = .
IN_HEADER = .

SRCS = $(wildcard $(SRC_D)/*.c)
OBJS = $(SRCS:$(SRC_D)/%.c=$(OBJ_D)/%.o)

BONUS_SRCS = $(wildcard $(SRC_D)/ft_lst*.c)
BONUS_OBJS = $(BONUS_SRCS:$(SRC_D)/%.c=$(OBJ_D)/%.o)

NAME = libft.a

all: $(NAME)

$(NAME): $(OBJS)
	ar rcs $(NAME) $(OBJS)

bonus: $(OBJS) $(BONUS_OBJS)
	ar rcs $(NAME) $(OBJS) $(BONUS_OBJS)

$(OBJ_D)/%.o : $(SRC_D)/%.c
	$(CC) $(CFLAGS) -I $(IN_HEADER) -c $< -o $@

clean:
	rm -f $(OBJS) $(BONUS_OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re bonus
