NAME := ircserv

CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98

INCDIR := ./include
SRCDIR := ./srcs
OBJDIR := ./objs

SRC := \
		tmp_ClientAdapter.cpp \
		ChannelRegistry.cpp \
		ClientRegistry.cpp \
		IrcParser.cpp \
		Signal.cpp \
		Error.cpp \
		Fd.cpp \
		Utils.cpp \
		Client.cpp \
		Server.cpp \
		IrcCore.cpp \
		main.cpp \

SRCS := $(addprefix $(SRCDIR)/, $(SRC))
OBJS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))
DEPS := $(OBJS:.o=.d)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) $(OBJS) -o $(NAME)

# srcs/xxx.cpp -> objs/xxx.o
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -MMD -MP -c $< -o $@

clean:
	rm -f $(OBJS) $(DEPS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

# 헤더 의존성 자동 포함 (처음엔 .d 없어도 에러 안 나게 -include)
-include $(DEPS)
