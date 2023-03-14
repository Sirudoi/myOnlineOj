COMPILE_SERVER=compile_server

cc=g++

$(COMPILE_SERVER):compile_server.cc
	$(cc) -o $@ $^ -std=c++11 -ljsoncpp -lpthread

.PHONY:clean
clean:
	rm -f $(COMPILE_SERVER)