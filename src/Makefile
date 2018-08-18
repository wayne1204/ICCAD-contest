CXX = g++
CFLAGS = -g -std=c++11
CFLAGS = -O3 -std=c++11

EXE = MainProgram
OBJ = layout.o polygon.o chipMgr.o util.o main.o
GRBPATH = gurobi801
LIB = libgurobi_c++.a


# Executable
all:$(OBJ) 
	$(CXX) -L$(GRBPATH)/linux64/lib -I$(GRBPATH)/linux64/include -o $(EXE) $(CFLAGS) $(OBJ) -lgurobi_c++ -lgurobi80
# all: $(OBJ)
# 	$(CXX) -o $(EXE) $(CFLAGS) $(OBJ) -L$(LIB) 

%.o:  %.cpp 
	$(CXX) -c $(CFLAGS) $< 

clean:
	rm $(OBJ) $(EXE)


