CXX = clang++

CXXFLAGS = -std=c++11 -g -Wall

#OSX
#CXXFLAGS += -stdlib=libc++

SRC = sequence.cpp main.cpp
OBJ = ${SRC:.cpp=.o}

first: seq

.cpp.o:
	${CXX} -c ${CXXFLAGS} -o $*.o $<

${OBJ}: sequence.h #all objects depend on the sequence.h file [templated]

seq: ${OBJ}
	${CXX} ${CXXFLAGS} -o seq ${OBJ}

clean:
	rm -rf seq ${OBJ}
