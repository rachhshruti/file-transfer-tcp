all:
	g++ -pthread -o Server WebServer.cpp
	g++ -o Client WebClient.cpp
clean:
	rm -f Server
	rm -f Client
