NAME    = brain

Cpp     = c++
CppFlags= -Wall -Wextra -Werror -std=c++98

SRCS    = main.cpp

OBJS    = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(Cpp) $(CppFlags) -o $(NAME) $(OBJS)

%.o: %.cpp
	$(Cpp) $(CppFlags) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
