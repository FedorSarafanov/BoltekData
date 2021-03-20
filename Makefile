CFLAGS=-O0


boltek-efm: boltek-efm.o Logger.o Writer.o Config.o BoltekTTY.o BoltekUSB.o
	g++ -o $@ $^ -lusb-1.0
	g++ BoltekEmulator.cpp -o BoltekEmulator -lutil

Logger.o: Logger.cpp
	g++ -c $^

Writer.o: Writer.cpp
	g++ -c $^

BoltekTTY.o: BoltekTTY.cpp
	g++ -c $^

BoltekUSB.o: BoltekUSB.cpp
	g++ -c $^
