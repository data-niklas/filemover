.POSIX:
.SUFFIXES:
CC     = gcc
LIBS=

CFLAGS = -Wall $(shell pkg-config --cflags $(LIBS)) -lm -lstdc++ -std=c++17 -fpermissive
LDLIBS = $(shell pkg-config --libs $(LIBS))
LDFLAGS = 
SRCS= filemover.cpp
OBJS=$(SRCS:.cpp=.o)
NAME=filemover

all: build

build: $(SRCS)
	$(CC) -o $(NAME) $^ $(CFLAGS) $(LDLIBS)	-O0

production: $(SRCS)
	$(CC) -o $(NAME) $^ $(CFLAGS) $(LDLIBS)	-O3


debug: $(SRCS)
	$(CC) -o $(NAME) $^ $(CFLAGS) -g $(LDLIBS) -O0

clean:
	rm -f $(wildcard *.o) $(NAME)
