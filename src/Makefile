CXX = g++
CFLAGS = -g -std=c++11
CFLAGS = -g -std=c++11

EXE = MainProgram
OBJ = main.o layout.o chipMgr.o util.o

# Executable
all:$(OBJ)
	$(CXX) -o $(EXE) $(CFLAGS) $(OBJ)

# Object file
%.o:  %.cpp
	$(CXX) -c $(CFLAGS) $<

clean:
	rm $(OBJ) $(EXE)


