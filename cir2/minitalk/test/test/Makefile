all: server client

server: server.c
	cc -Wall -Wextra -Werror server.c -o server

client: client.c
	cc -Wall -Wextra -Werror client.c -o client

clean:
	rm -f server client

fclean: clean

re: fclean all
