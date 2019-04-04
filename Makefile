NAME=mbx
SRC=main.c
OBJ=$(SRC:.c=.o)
CFLAGS=-Wall -Werror
LDFLAGS=-lncurses -lform

LD=$(CC)
RM=rm -fr

all: $(NAME)

$(NAME): $(OBJ)
	$(LD) $(LDFLAGS) -o $(NAME) $(OBJ)

clean:
	$(RM) $(OBJ) $(NAME)

re: clean all
