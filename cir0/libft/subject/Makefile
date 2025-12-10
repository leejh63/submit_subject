CC = cc
CFLAGS = -Wall -Wextra -Werror
SRC_D = .
OBJ_D = .
IN_HEADER = .
SRCS = $(SRC_D)/ft_atoi.c \
       $(SRC_D)/ft_bzero.c \
       $(SRC_D)/ft_calloc.c \
       $(SRC_D)/ft_isalnum.c \
       $(SRC_D)/ft_isalpha.c \
       $(SRC_D)/ft_isascii.c \
       $(SRC_D)/ft_isdigit.c \
       $(SRC_D)/ft_isprint.c \
       $(SRC_D)/ft_itoa.c \
       $(SRC_D)/ft_memchr.c \
       $(SRC_D)/ft_memcmp.c \
       $(SRC_D)/ft_memcpy.c \
       $(SRC_D)/ft_memmove.c \
       $(SRC_D)/ft_memset.c \
       $(SRC_D)/ft_putchar_fd.c \
       $(SRC_D)/ft_putendl_fd.c \
       $(SRC_D)/ft_putnbr_fd.c \
       $(SRC_D)/ft_putstr_fd.c \
       $(SRC_D)/ft_split.c \
       $(SRC_D)/ft_strchr.c \
       $(SRC_D)/ft_strdup.c \
       $(SRC_D)/ft_striteri.c \
       $(SRC_D)/ft_strjoin.c \
       $(SRC_D)/ft_strlcat.c \
       $(SRC_D)/ft_strlcpy.c \
       $(SRC_D)/ft_strlen.c \
       $(SRC_D)/ft_strmapi.c \
       $(SRC_D)/ft_strncmp.c \
       $(SRC_D)/ft_strnstr.c \
       $(SRC_D)/ft_strrchr.c \
       $(SRC_D)/ft_strtrim.c \
       $(SRC_D)/ft_substr.c \
       $(SRC_D)/ft_tolower.c \
       $(SRC_D)/ft_toupper.c

OBJS = $(SRCS:$(SRC_D)/%.c=$(OBJ_D)/%.o)
BONUS_SRCS = $(SRC_D)/ft_lstadd_back_bonus.c \
             $(SRC_D)/ft_lstadd_front_bonus.c \
             $(SRC_D)/ft_lstclear_bonus.c \
             $(SRC_D)/ft_lstdelone_bonus.c \
             $(SRC_D)/ft_lstiter_bonus.c \
             $(SRC_D)/ft_lstlast_bonus.c \
             $(SRC_D)/ft_lstmap_bonus.c \
             $(SRC_D)/ft_lstnew_bonus.c \
             $(SRC_D)/ft_lstsize_bonus.c

BONUS_OBJS = $(BONUS_SRCS:$(SRC_D)/%.c=$(OBJ_D)/%.o)

NAME = libft.a

all: $(NAME)

$(NAME): $(OBJS)
	ar rcs $(NAME) $(OBJS)

$(OBJ_D)/%.o: $(SRC_D)/%.c
	$(CC) $(CFLAGS) -I $(IN_HEADER) -c $< -o $@

bonus: $(OBJS) $(BONUS_OBJS)
	ar rcs $(NAME) $(OBJS) $(BONUS_OBJS)
	touch .bonus

clean:
	rm -f $(OBJS) $(BONUS_OBJS) .bonus

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re bonus
