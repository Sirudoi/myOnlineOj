COMPLIE_SERVER=complie_server

cc=g++

$(COMPLIE_SERVER):complie_server.cc
	$(cc) -o $@ $^ -std=c++11 -ljsoncpp

.PHONY:clean
clean:
	rm -f $(COMPLIE_SERVER)