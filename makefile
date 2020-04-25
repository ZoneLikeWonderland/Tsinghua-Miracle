G++FLAG=-O3 -std=c++17

main.exe: gameunit.o calculator.o ai_client.o ai.o
	g++ $(G++FLAG) gameunit.o calculator.o ai_client.o ai.o -o main.exe

gameunit.o: gameunit.cpp
	g++ $(G++FLAG) -c gameunit.cpp -o gameunit.o

calculator.o: calculator.cpp
	g++ $(G++FLAG) -c calculator.cpp -o calculator.o

ai_client.o: ai_client.cpp
	g++ $(G++FLAG) -c ai_client.cpp -o ai_client.o
	
ai.o: ai.cpp
	g++ $(G++FLAG) -c ai.cpp -o ai.o

