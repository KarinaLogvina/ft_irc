NAME		= ircserv

SRCS		= ircserv.cpp

OBJS		= $(SRCS:.cpp=.o)

#HEADER		= PmergeMe.hpp

CC			= c++

CFLAGS		= -Wall -Wextra -Werror -std=c++98

RM			= rm -f

all:		$(NAME)

$(NAME):	$(OBJS)
			$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o:		%.cpp $(HEADER)
			$(CC) $(CFLAGS) -c $<

clean:
			$(RM) $(OBJS)

fclean:		clean
			$(RM) $(NAME)

re:			fclean all

.PHONY:		all clean fclean re