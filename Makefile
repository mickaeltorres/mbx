NAME=mbx
SRC=main.c
OBJ=$(SRC:.c=.o)
CFLAGS=-Wall -Werror
LDLOCALFLAGS=`uname | grep -q Linux && echo -lbsd`
LDFLAGS=-lncurses -lform $(LDLOCALFLAGS)

LD=$(CC)
RM=rm -fr

all: $(NAME)

$(NAME): $(OBJ)
	$(LD) -o $(NAME) $(OBJ) $(LDFLAGS)

clean:
	$(RM) $(OBJ) $(NAME)

re: clean all
