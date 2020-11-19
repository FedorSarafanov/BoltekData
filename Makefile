CFLAGS=-O0

get_data: get_data.o logger.o
	g++ -o $@ $^ -lusb-1.0

get_data.o: get_data.cpp
	g++ -c $^

logger.o: logger.cpp
	g++ -c $^
