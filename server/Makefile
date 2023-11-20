CC = gcc
NAME = aesdsocket
CFLAGS = -Wall -O2
SRC = *.c
OBJS = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CROSS_COMPILE)$(CC) $(OBJS) $(CFLAGS) -o $@

%.o: %.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -c  $< -o $@

clean:
	rm -f *.o
	rm -f *~
	rm -f $(NAME)
