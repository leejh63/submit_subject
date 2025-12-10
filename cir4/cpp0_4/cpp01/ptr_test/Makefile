NAME		= Harl
CPP			= c++
CPPFLAGS	= -Wall -Wextra -Werror -std=c++98 -g -O0 -fno-inline -fno-omit-frame-pointer

SRCS		= \
		main.cpp \
		Harl.cpp \

HEADS		= \
		Harl.hpp \
	
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
