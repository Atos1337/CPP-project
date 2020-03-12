all: Sign
	
Sign: src/Sign_example.cpp obj/ZIP-file_signing.o obj/ZIP-file_reading.o obj/ZIP-file_writing.o
	g++ -std=c++17 -Iinclude -o Sign -Wall -Wextra -Werror src/Sign_example.cpp obj/ZIP-file_signing.o obj/ZIP-file_reading.o obj/ZIP-file_writing.o -lz

clean: 
	rm -f Sign
	rm -rf obj

obj: 
	mkdir obj

obj/ZIP-file_reading.o: obj src/ZIP-file_reading.cpp 
	g++ -std=c++17 -Iinclude -c -Wall -Wextra -Werror src/ZIP-file_reading.cpp -o obj/ZIP-file_reading.o -lz
	
obj/ZIP-file_writing.o: obj src/ZIP-file_writing.cpp
	g++ -std=c++17 -Iinclude -c -Wall -Wextra -Werror src/ZIP-file_writing.cpp -o obj/ZIP-file_writing.o -lz
	
obj/ZIP-file_signing.o: obj obj/ZIP-file_writing.o obj/ZIP-file_reading.o
	g++ -std=c++17 -Iinclude -c -Wall -Wextra -Werror src/ZIP-file_signing.cpp -o obj/ZIP-file_signing.o -lz
