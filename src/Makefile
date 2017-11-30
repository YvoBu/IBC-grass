PROJ=ibcgrass

SRC=Cell.cpp\
    Environment.cpp\
    Genet.cpp\
    Grid.cpp\
    GridEnvir.cpp\
    IBC-grass.cpp\
    Output.cpp\
    Parameters.cpp\
    Plant.cpp\
    RandomGenerator.cpp\
    Seed.cpp\
    Traits.cpp\
    CThread.cpp\
    CSimulation.cpp\
    mycorrhiza.cpp

OBJ=$(SRC:.cpp=.o)

CXXFLAGS+=-O3 -g -Wall -std=c++14
LDFLAGS=-lpthread -g

all : ibcgrass

ibcgrass : depend $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) -o $(PROJ)

clean:
	$(RM) -f $(PROJ)
	$(RM) -f $(OBJ)
	$(RM) -f depend

depend : $(SRC)
	$(CXX) $(CXXFLAGS) -MM $(SRC) > depend

-include depend
