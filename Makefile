CXXFLAGS = -std=c++11

SRC = sequence.cpp
OBJ = ${SRC:.cpp=.o}

.cpp.o:
	${CXX} -c ${CXXFLAGS} -o $*.o $<

seq: ${OBJ}
	${CXX} -o seq ${OBJ}

clean:
	rm -rf seq ${OBJ}
