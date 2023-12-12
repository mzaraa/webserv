NAME		= webserv
SRCS		= srcs/main.cpp \
			srcs/Server.cpp \
			srcs/Location.cpp \
			srcs/ConfigParser.cpp \
			srcs/Utils.cpp \
			srcs/Socket.cpp \
			srcs/Client.cpp \
			srcs/Request.cpp \
			srcs/Response.cpp \
			srcs/WebservManager.cpp \
			srcs/Cgi.cpp 
HEADERS		=
CC			= g++
CPP_FLAGS	= -std=c++98 -Wall -Wextra -Werror
LD			= $(CC)
OBJS		= $(SRCS:%.cpp=%.o)
RM			= rm -rf

all: $(NAME)

$(NAME): $(OBJS)
	@$(LD) $(OBJS) -o $(NAME)
	@printf "$(NAME)\n"

%.o: %.cpp $(HEADERS)
	@$(CC) -c $(CPP_FLAGS) $< -o $@
	@printf "$<\n"

clean:
	@$(RM) $(OBJS)

fclean: clean
	@$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re