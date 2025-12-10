# **************************************************************************** #
#                        get_next_line  (split build)                           #
# **************************************************************************** #

# ── 설정 ─────────────────────────────────────────────────────────────────────
NAME        := get_next_line.a
CC          := cc
AR          := ar rcs
RM          := rm -f
CFLAGS      := -Wall -Wextra -Werror
BUFFER_SIZE ?= 1024
CFLAGS      += -D BUFFER_SIZE=$(BUFFER_SIZE)

# ── 필수 소스 / 오브젝트 ─────────────────────────────────────────────────────
SRCS        := get_next_line.c get_next_line_utils.c
OBJS        := $(SRCS:.c=.o)

# ── 보너스 소스 / 오브젝트 (독립 구현) ──────────────────────────────────────
BONUS_SRCS  := get_next_line_bonus.c get_next_line_utils_bonus.c
BONUS_OBJS  := $(BONUS_SRCS:.c=.o)

# ── 기본(필수) 빌드 ─────────────────────────────────────────────────────────
all: $(NAME)

$(NAME): $(OBJS)
	$(AR) $@ $^

# 각 .c → .o (필수 헤더 의존)
%.o: %.c get_next_line.h
	$(CC) $(CFLAGS) -c $< -o $@

# ── 보너스 빌드 : 필수 .o 무시, 보너스만 아카이브 ──────────────────────────
bonus: $(BONUS_OBJS)
	$(AR) $(NAME) $(BONUS_OBJS)

# 보너스 객체는 보너스 헤더에 의존
$(BONUS_OBJS): %.o: %.c get_next_line_bonus.h
	$(CC) $(CFLAGS) -c $< -o $@

# ── 정리 규칙 ───────────────────────────────────────────────────────────────
clean:
	$(RM) $(OBJS) $(BONUS_OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all bonus clean fclean re

