CFLAGS=-O0

test_boltek: test_boltek.o logger.o writer.o boltek.o
	g++ -o $@ $^ -lusb-1.0



get_data: get_data.o logger.o writer.o boltek.o
	g++ -o $@ $^ -lusb-1.0

get_data.o: get_data.cpp
	g++ -c $^

logger.o: logger.cpp
	g++ -c $^

writer.o: writer.cpp
	g++ -c $^

boltek.o: boltek.cpp
	g++ -c $^

test_boltek.o: test_boltek.cpp
	g++ -c $^


