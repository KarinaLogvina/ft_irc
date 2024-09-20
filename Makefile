NAME		= ircserv

SRCS_DIR	= src/

COMMANDS_DIR = commands/

OBJS_DIR	= objs/

SRCS		= main.cpp Server.cpp Client.cpp Channel.cpp utils.cpp

COMMAND_SRCS = Invite.cpp Join.cpp Kick.cpp Quit.cpp Topic.cpp Mode.cpp PrivMSG.cpp Part.cpp

ALL_SRCS 	= $(SRCS:%=$(SRCS_DIR)%) $(COMMAND_SRCS:%=$(COMMANDS_DIR)%)

OBJS		= $(ALL_SRCS:%.cpp=$(OBJS_DIR)%.o)

CC			= c++

CFLAGS		= -Wall -Wextra -Werror -MMD -MP -std=c++98 -I includes/

RM			= rm -f

all:		$(NAME)

$(NAME):	$(OBJS)
			$(CC) $(CFLAGS) $^ -o $@

$(OBJS_DIR)%.o: %.cpp
			@mkdir -p $(dir $@)
			$(CC) $(CFLAGS) -c $< -o $@

-include $(OBJS:%.o=%.d)

clean:
			$(RM) -r $(OBJS_DIR)

fclean:		clean
			$(RM) $(NAME)

re:			fclean all

.PHONY:		all clean fclean re