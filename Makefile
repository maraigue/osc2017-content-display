CC=g++
CCFLAGS=-std=c++11 -DBOOST_ALL_NO_LIB=1 -lboost_system -lboost_filesystem -lpthread

all: server

connection.o: connection.hpp connection_manager.hpp request_handler.hpp
connection_manager.o: connection_manager.hpp
main.o: server.hpp
mime_types.o: mime_types.hpp
reply.o: reply.hpp
request_handler.o: request_handler.hpp mime_types.hpp reply.hpp request.hpp
request_parser.o: request_parser.hpp request.hpp
server.o: server.hpp

server: connection.o connection_manager.o main.o mime_types.o reply.o request_handler.o request_parser.o server.o
	$(CC) $^ -o $@ $(CCFLAGS)

.cpp.o:
	$(CC) -c $< $(CCFLAGS)

clean:
	rm -vf *.o
	rm -vf server

