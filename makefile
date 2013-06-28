CC=g++
CFLAGS=-O3
OBJECTS=glVector3.o terrain.o camera.o
LIBS = -lSDL -lGL -lGLU

all:    terrain
terrain: $(OBJECTS) 
	$(CC) src/main.cpp $(OBJECTS) $(LIBS) $(CFLAGS) -o main
        
terrain.o: src/terrain.cpp src/terrain.h
	$(CC) -c src/terrain.cpp $(CFLAGS)
       
camera.o: src/camera.cpp src/camera.h
	$(CC) -c src/camera.cpp $(CFLAGS)
	   
glVector3.o: src/glVector3.cpp src/glVector3.h
	$(CC) -c src/glVector3.cpp $(CFLAGS)
	
clean:
	rm *.o main