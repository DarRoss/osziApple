SRCS = $(wildcard *.cpp)
PROGS = $(patsubst %.cpp,%,$(SRCS))

CC = g++
CFLAGS = -Wall -Wextra -Werror -g

all : $(PROGS)

% : %.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(PROGS)
