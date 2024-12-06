PROG = locations
CC = clang

all: $(PROG)

$(PROG): main.o
	$(CC) -o $(PROG) $<

%.o:%.c
	$(CC) -c -Werror -Wall -pedantic -g -O0 -o $@ $<

clean:
	@rm -vf *.o  $(PROG)
