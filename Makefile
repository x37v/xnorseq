CXX = clang++

CXXFLAGS = -std=c++11 -g -Wall

#OSX
#CXXFLAGS += -stdlib=libc++

SRC = sequence.cpp main.cpp
OBJ = ${SRC:.cpp=.o}

.cpp.o:
	${CXX} -c ${CXXFLAGS} -o $*.o $<

seq: ${OBJ} sequence.h
	${CXX} ${CXXFLAGS} -o seq ${OBJ}

clean:
	rm -rf seq ${OBJ}
