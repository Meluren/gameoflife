output: src/main.o
	gcc src/main.o -lSDL2 -Wall -o life

main.o: src/main.c
	gcc src/main.o -c -lSDL2 -Wall

clean:
	rm src/*.o life
