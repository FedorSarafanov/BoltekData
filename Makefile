CFLAGS=-O0

get_data: get_data.o
	gcc -o $@ $^ -lusb-1.0
	
