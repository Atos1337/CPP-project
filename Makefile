all: Sign
	
Sign: src/Sign_example.cpp obj/ZIP_file_signing.o obj/ZIP_file_reading.o obj/ZIP_file_writing.o obj/Test_function_for_sign.o
	g++ -std=c++17 -Iinclude -o Sign -Wall -Wextra -Werror src/Sign_example.cpp obj/ZIP_file_signing.o obj/ZIP_file_reading.o obj/ZIP_file_writing.o obj/Test_function_for_sign.o -lz -lssl -lcrypto -lstdc++fs

clean: 
	rm -f Sign
	rm -rf obj

obj: 
	mkdir obj
	
obj/Test_function_for_sign.o: obj src/Test_function_for_sign.cpp
	g++ -std=c++17 -Iinclude -c -Wall -Wextra -Werror src/Test_function_for_sign.cpp -o obj/Test_function_for_sign.o

obj/ZIP_file_reading.o: obj src/ZIP_file_reading.cpp 
	g++ -std=c++17 -Iinclude -c -Wall -Wextra -Werror src/ZIP_file_reading.cpp -o obj/ZIP_file_reading.o -lz -lssl -lcrypto
	
obj/ZIP_file_writing.o: obj src/ZIP_file_writing.cpp
	g++ -std=c++17 -Iinclude -c -Wall -Wextra -Werror src/ZIP_file_writing.cpp -o obj/ZIP_file_writing.o -lssl -lcrypto -lz
	
obj/ZIP_file_signing.o: obj obj/ZIP_file_writing.o obj/ZIP_file_reading.o obj/Test_function_for_sign.o
	g++ -std=c++17 -Iinclude -c -Wall -Wextra -Werror src/ZIP_file_signing.cpp -o obj/ZIP_file_signing.o -lz -lstdc++fs
