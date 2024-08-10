NAME		= ircserv

SRCS_DIR	= src/

OBJS_DIR	= objs/

SRCS		= main.cpp Server.cpp Client.cpp Channel.cpp utils.cpp

OBJS		= $(SRCS:%.cpp=$(OBJS_DIR)%.o)

CC			= c++

CFLAGS		= -Wall -Wextra -Werror -MMD -MP -std=c++98 -I includes/

RM			= rm -f

all:		$(NAME)

$(NAME):	$(OBJS)
			$(CC) $(CFLAGS) $^ -o $@

$(OBJS):	$(OBJS_DIR)%.o: ${SRCS_DIR}%.cpp
			@mkdir -p $(dir $@)
			$(CC) $(CFLAGS) -c $< -o $@

clean:
			$(RM) -r $(OBJS_DIR)

fclean:		clean
			$(RM) $(NAME)

re:			fclean all

.PHONY:		all clean fclean re