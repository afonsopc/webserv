CPP = c++
CFLAGS = -g -Wall -Wextra -Werror -std=c++98
NAME = webserv
SRCS = main.cpp src/ServerSocket.cpp src/ClientSocket.cpp src/HttpRequest.cpp src/HttpResponse.cpp src/Config.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CPP) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CPP) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re