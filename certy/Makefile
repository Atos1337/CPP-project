all: obj print_cert ssign

print_cert:obj/main.o 
	g++ -Wall -Wextra obj/main.o -o print_cert -lssl -lcrypto

ssign:obj/signer.o
	g++ -Wall -Wextra obj/signer.o -o ssign -lssl -lcrypto
obj/main.o:main.cpp 
	g++ -std=c++17 -c -Wall -Wextra  main.cpp -o obj/main.o -lssl -lcrypto

obj/signer.o:signer.cpp signer.h
	g++ -std=c++17 -c -Wall -Wextra  signer.cpp -o obj/signer.o -lssl -lcrypto
obj:
	mkdir -p obj

clean:
	rm -rf obj print_cert ssign