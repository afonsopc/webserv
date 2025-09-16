NAME = webserv
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
INCLUDES = -I headers
SRCS = $(shell find src -name "**.cpp")
OBJ_DIR = obj
OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))

all: $(NAME)

$(NAME): $(OBJS)
	@echo "\033[1;32mCompiling \033[1;0m\"$(OBJS)\"\033[1;32m into \033[1;0m\"$(NAME)\"\033[1;32m.\033[0m"
	@$(CC) -o $(NAME) $(OBJS) $(CFLAGS) $(INCLUDES)

$(OBJ_DIR)/%.o: %.cpp
	@echo "\033[1;32mCompiling \033[1;0m\"$<\"\033[1;32m into \033[1;0m\"$@\"\033[1;32m.\033[0m"
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@rm -rf $(OBJ_DIR)

fclean: clean
	@rm -rf $(NAME)

re: fclean all

.PHONY: all re clean fclean
