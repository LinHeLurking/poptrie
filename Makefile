dbgall:
	g++ -g -Wall -std=c++11 -fsanitize=address ftb.cpp main.cpp poptrie.cpp prefix_tree.cpp -Iinclude/ -o test

all:
	g++ -O1 -Wall ftb.cpp main.cpp poptrie.cpp prefix_tree.cpp -Iinclude/ -o main
