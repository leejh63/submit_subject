NAME	= seq

Cpp	= c++

Cppflags= -std=c++98 -Wall -Wextra -Werror

SRCS	= \
	main.cpp \


HEADS	= \

	
OBJS = $(SRCS:.cpp=.o)

all	: $(NAME)

$(NAME)	: $(OBJS)
	$(Cpp) $(Cppflags) $(OBJS) -o $(NAME) 
	
%.o : %.cpp $(HEADS)
	$(Cpp) $(Cppflags) -c $< -o $@

clean	:
	rm -f $(OBJS)

fclean	: clean
	rm -f $(NAME)

re: fclean all
