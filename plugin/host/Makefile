.PHONY: all clean

CXXFLAGS=-c -Wall -Wextra -Werror -Iinclude -g -std=c++17

all: Native-messaging-host

Native-messaging-host: obj/Native-messaging-host.o
	g++ obj/Native-messaging-host.o -o Native-messaging-host -g

obj: 
	mkdir obj

obj/Native-messaging-host.o: src/Native-messaging-host.cpp include/portable-file-dialogs.h | obj
	g++ $(CXXFLAGS) src/Native-messaging-host.cpp -o obj/Native-messaging-host.o

clean:
	rm obj Native-messaging-host -rf