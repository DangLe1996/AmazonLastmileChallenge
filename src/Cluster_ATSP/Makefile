
CC = gcc -fPIC -std=c11 -O2


OBJ= LKheuristic.o read_data.o

ATSP1: $(OBJ)
	$(CXX) -shared $(OBJ) -o ATSP.so 

read_data.o: read_data.c headers.h
	$(CC) -c read_data.c

LKheuristic.o : LKheuristic.c headers.h
	$(CC) -c LKheuristic.c
clean :
	rm ATSP.so $(OBJ)

