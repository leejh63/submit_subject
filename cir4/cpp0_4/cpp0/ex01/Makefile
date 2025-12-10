NAME    = phonebook

CPP     = c++
CPPFLAGS= -Wall -Wextra -Werror -std=c++98

SRCS    = \
	main.cpp \
	Contact.cpp \
	PhoneBook.cpp

OBJS    = $(SRCS:.cpp=.o)

DEPS	= $(OBJS:.o=.d)

all: $(NAME)

$(NAME): $(OBJS)
	$(CPP) $(CPPFLAGS) $(OBJS) -o $(NAME) 

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

clean:
	rm -f $(OBJS) $(DEPS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
