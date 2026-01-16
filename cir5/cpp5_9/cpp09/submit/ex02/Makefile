NAME 		:= PmergeMe

CPP 		:= c++ 
CPPFLAGS 	:= -Wall -Wextra -Werror -std=c++98

SRCS 		:= \
				main.cpp \
				PmergeMe.cpp \

HEADS		:= \
				PmergeMe.hpp \

OBJS 		:= $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CPP) $(CPPFLAGS) -o $@ $^

%.o: %.cpp $(HEADS)
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
