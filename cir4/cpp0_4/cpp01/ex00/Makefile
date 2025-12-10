NAME    = Zombie

Cpp     = c++
CppFlags= -Wall -Wextra -Werror -std=c++98

SRCS    = \
	main.cpp \
	Zombie.cpp \
	newZombie.cpp \
	randomChump.cpp \

HEADERS = \
	Zombie.hpp \

OBJS    = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(Cpp) $(CppFlags) -o $(NAME) $(OBJS)

%.o: %.cpp $(HEADERS)
	$(Cpp) $(CppFlags) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
