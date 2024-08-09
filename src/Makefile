NAME		= ircserv

SRCS		= main.cpp Server.cpp Client.cpp Channel.cpp utils.cpp

OBJS		= $(SRCS:.cpp=.o)

HEADER		= Server.hpp Client.hpp Channel.hpp response.hpp utils.hpp

HEADER_DIR  = ../includes/

HEADERS		= $(addprefix ../includes/, $(HEADER)) 

CC			= c++

CFLAGS		= -Wall -Wextra -Werror -std=c++98

RM			= rm -f

all:		$(NAME)

$(NAME):	$(OBJS) $(HEADERS)
			$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o:		%.cpp 
			$(CC) $(CFLAGS) -c $<

clean:
			$(RM) $(OBJS)

fclean:		clean
			$(RM) $(NAME)

re:			fclean all

.PHONY:		all clean fclean re