all: testzip

testzip: src/main.cpp obj/ZIP-file_reading.o
	g++ -std=c++17 -Iinclude -o testzip -Wall -Wextra -Werror src/main.cpp obj/ZIP-file_reading.o -lz

clean: 
	rm -f testzip
	rm -rf obj

obj: 
	mkdir obj

obj/ZIP-file_reading.o: obj src/ZIP-file_reading.cpp 
	g++ -std=c++17 -Iinclude -c -Wall -Wextra -Werror src/ZIP-file_reading.cpp -o obj/ZIP-file_reading.o -lz
