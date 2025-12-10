CC		= cc
CFLAGS	= -g -Wall -Wextra -Werror -I.

NAME		= philo

HEADER	= \
	philo.h

SRCS		= \
	main.c \
	ft_atoi.c \
	print_func.c \
	time_func.c \
	set_func.c \
	lock_func.c \
	do_func.c

OBJS		= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
