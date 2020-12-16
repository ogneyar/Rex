all:
	g++ -O3 -s server.cpp -l ws2_32

clean:
	del -f a.exe

s:
	g++ -O3 -s server.cpp -o server.exe -l ws2_32
	
server:
	g++ -O3 -s server.cpp -o server.exe -l ws2_32



