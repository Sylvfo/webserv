# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: syl <syl@student.42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/02/12 08:32:46 by cmegret           #+#    #+#              #
#    Updated: 2025/08/28 14:35:54 by syl              ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

#colors
DARK_BLUE = \033[38;5;18m
DARK_CYAN = \033[38;5;30m
DARK_GREEN = \033[38;5;28m
BRIGHT_BLUE = \033[38;5;75m
LIGHT_BLUE = \033[38;5;153m
YELLOW = \033[0;33m
BRIGHT_IVORY = \033[38;5;230m
SOFT_ORANGE = \033[38;5;215m
RED = \033[0;31m
RESET = \033[0m
BOLD = \033[1m
UNDERLINE = \033[4m

NAME = webserv
#NAME2 = webserv_bonus
CC = c++

CFLAGS = -Wall -Wextra -Werror -std=c++98 #-g

# Fichiers sources
INC_DIR = inc/
SRC_DIR = src/
SRCS = ${shell find ${SRC_DIR} -type f -name '*.cpp'}
DEBUG = $(SRC_DIR)/debug/

# Fichiers objets
OBJ_DIR = obj
OBJ = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)
#OBJ_BONUS = $(SRCS_BONUS:%.cpp=$(OBJ_DIR)/%.o)

all: $(NAME)

# Règle pour l'exécutable final
$(NAME): $(OBJ)
	@echo "$(DARK_CYAN)Linking: $@$(RESET)"
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJ)
	@echo "$(DARK_GREEN)\nCompilation successful!\n$(RESET)"
	@echo "$(DARK_BLUE)$(BOLD)               ',        ,'    $(RESET)"
	@echo "$(BRIGHT_BLUE)$(BOLD)               ¡'\      , ¡    $(RESET)"
	@echo "$(DARK_CYAN)$(BOLD)   Piou!      $(LIGHT_BLUE) \.'\. ./' -!    $(RESET)"
	@echo "$(LIGHT_BLUE)$(BOLD)           /,   $(UNDERLINE)\  ,'$(RESET)$(LIGHT_BLUE)/'-./     //    $(RESET)"
	@echo "$(YELLOW)$(BOLD)        <$(RESET)(°')~-"'/'.-:-,-:"'.\_/=<'<'~-   $(RESET)"
	@echo "$(BRIGHT_IVORY)$(BOLD)                \ $(UNDERLINE)'°° °'$(RESET)/"
	@echo "$(SOFT_ORANGE)$(BOLD)                   ! ¡          $(RESET)"
	@echo "$(SOFT_ORANGE)$(BOLD)                   + ÷          $(RESET)"
	@echo "$(DARK_GREEN)$(BOLD)                  [$(YELLOW)¡$(DARK_GREEN)!$(YELLOW)¡$(DARK_GREEN)]        $(RESET)"
	@echo "$(DARK_CYAN)$(BOLD)                  [$(YELLOW)¡$(DARK_CYAN)!$(YELLOW)¡$(DARK_CYAN)]        \n\n$(RESET)"

# Règle pour les fichiers objets
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@
	@echo "$(DARK_CYAN)Compiling: $<$(RESET)"
	
#a revoir...
debug: CFLAGS += -g
debug: re

# Règle pour nettoyer les fichiers objets
clean:
	@echo "$(RED)Cleaning object files...$(RESET)"
	@rm -rf $(OBJ_DIR)

# Règle pour nettoyer tout
fclean: clean
	@echo "$(RED)Cleaning executable(s)...$(RESET)"
	@rm -f $(NAME)
	#@rm -f $(NAME2)
	@rm -f $(TEST_NAME)
	clear

# Règle pour refaire tout
re: fclean all

# Règle pour rendre le makefile "phony"
.PHONY: all clean fclean re debug