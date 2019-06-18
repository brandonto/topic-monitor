all:
	g++ -std=c++11 -Wl,-rpath=/usr/local/lib -o topic-monitor main.cpp solClientThread.cpp monitoringThread.cpp utils.cpp -lssl -lcrypto -lpthread -lsolclient -llua5.2

debug:
	g++ -g -std=c++11 -Wl,-rpath=/usr/local/lib -o topic-monitor main.cpp solClientThread.cpp monitoringThread.cpp utils.cpp -lssl -lcrypto -lpthread -lsolclient -llua5.2
