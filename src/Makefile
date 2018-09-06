CXX = g++
CFLAGS = -g -std=c++11 
#CFLAGS = -O3 -std=c++11

EXE = cada045
OBJ = main.o chipMgr.o prelayout.o layout.o polygon.o parseFile.o util.o 
GRBPATH = gurobi801

# Executable
all:$(OBJ) 
	# $(CXX) -L$(GRBPATH)/linux64/lib -I$(GRBPATH)/linux64/include -o $(EXE) $(CFLAGS) $(OBJ) -lgurobi_c++ -lgurobi80
	$(CXX) -Llib -Iinclude -o $(EXE) $(CFLAGS) $(OBJ) -lgurobi_c++ -lgurobi80

# Object file
%.o:  %.cpp
	$(CXX) -c $(CFLAGS) $<

clean:
	rm $(OBJ) $(EXE)


