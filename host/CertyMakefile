all: obj/signer.o

example: obj/signer.o obj/example.o | obj
	g++ -Wall -Wextra obj/signer.o obj/example.o -o example -lssl -lcrypto

obj/example.o: example.cpp | obj
	g++ -std=c++17 -c -Wall -Wextra  example.cpp -o obj/example.o -lssl -lcrypto

obj/signer.o: signer.cpp signer.h | obj
	g++ -std=c++17 -c -Wall -Wextra  signer.cpp -o obj/signer.o -lssl -lcrypto

obj:
	mkdir -p obj

clean:
	rm -rf obj example