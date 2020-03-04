all: obj print_cert

print_cert:obj/main.o 
	g++ -Wall -Werror obj/main.o -o print_cert -lssl -lcrypto

obj/main.o:main.cpp 
	g++ -std=c++17 -c -Wall -Wextra -Werror main.cpp -o obj/main.o -lssl -lcrypto


obj:
	mkdir -p obj

clean:
	rm -rf obj print_cert