NAME = webserv
CXX = c++
CC = cc
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g
CFLAGS = -Wall -Wextra -Werror
INCLUDES = -I headers
CPP_SRCS = $(shell find src -name "*.cpp")
C_SRCS = $(shell find src -name "*.c")
OBJ_DIR = obj
CPP_OBJS = $(addprefix $(OBJ_DIR)/, $(CPP_SRCS:.cpp=.o))
C_OBJS = $(addprefix $(OBJ_DIR)/, $(C_SRCS:.c=.o))
OBJS = $(CPP_OBJS) $(C_OBJS)

all: $(NAME)

$(NAME): $(OBJS)
	@printf "\033[1;32mCompiling \033[1;0m\"$(OBJS)\"\033[1;32m into \033[1;0m\"$(NAME)\"\033[1;32m.\033[0m\n"
	@$(CXX) -o $(NAME) $(OBJS) $(CXXFLAGS) $(INCLUDES)

$(OBJ_DIR)/%.o: %.cpp
	@printf "\033[1;32mCompiling \033[1;0m\"$<\"\033[1;32m into \033[1;0m\"$@\"\033[1;32m.\033[0m\n"
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: %.c
	@printf "\033[1;32mCompiling \033[1;0m\"$<\"\033[1;32m into \033[1;0m\"$@\"\033[1;32m.\033[0m\n"
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@rm -rf $(OBJ_DIR)

fclean: clean
	@rm -rf $(NAME)

re: fclean all

.PHONY: all re clean fclean
