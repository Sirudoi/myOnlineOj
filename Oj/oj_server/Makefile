OJ_SERVER=oj_server
cc=g++

$(OJ_SERVER):oj_server.cc
	$(cc) -o $@ $^ -std=c++11 -lpthread -lctemplate -ljsoncpp

.PHONY:clean
clean:
	rm -f $(OJ_SERVER)