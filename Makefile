CXX = clang++
#CXXFLAGS = -g -Wall -std=c++1y -stdlib=libstdc++
CXXFLAGS = -g -Wall -std=c++11

#OSX
#CXXFLAGS += -stdlib=libc++

SRC = main.cpp sequence.cpp
OBJ = ${SRC:.cpp=.o}

first: seq

.cpp.o:
	${CXX} -c ${CXXFLAGS} -o $*.o $<

${OBJ}: sequence.h #all objects depend on the sequence.h file [templated]

seq: ${OBJ}
	${CXX} ${CXXFLAGS} -o seq ${OBJ}

clean:
	rm -rf seq ${OBJ}
