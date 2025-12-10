NAME     = template
CPP      = c++
CPPFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS		= \
		main.cpp \


HEADS		= \
		iter.hpp \

	
OBJS		= $(SRCS:.cpp=.o)

all		: $(NAME)

$(NAME) : $(OBJS)
	$(CPP) $(CPPFLAGS) $(OBJS) -o $(NAME)

%.o 	: %.cpp $(HEADS)
	$(CPP) $(CPPFLAGS) -c $< -o $@

clean	:
	rm -f $(OBJS)

fclean	: clean
	rm -f $(NAME)

re		: fclean all
