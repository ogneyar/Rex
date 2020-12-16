all:
	g++ -O3 -s server.cpp -o rex.exe -l ws2_32

clean:
	del -f server.exe

32:
	g++ -O3 -s server.cpp -o exe/rex32.exe -l ws2_32
	
64:
	g++ -O3 -s server.cpp -o exe/rex64.exe -l ws2_32



