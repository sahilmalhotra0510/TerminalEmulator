CC=gcc
CFLAGS=-lreadline -I. -Wall
DEPS = prompt.h input.h tokenize.h execute.h cdbuiltin.h pwdbuiltin.h echobuiltin.h lsbuiltin.h nightswatch.h pinfo.h jobcontrol.h
OBJ = prompt.o shell.o input.o tokenize.o execute.o cdbuiltin.o pwdbuiltin.o echobuiltin.o lsbuiltin.o nightswatch.o pinfo.o jobcontrol.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

shell: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o *~ core
