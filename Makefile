CXX = clang++

#LINUX
CXXFLAGS = -std=c++11 -g -Wall

#OSX
#CXXFLAGS = -std=c++11 -g -Wall -stdlib=libc++

SRC = sequence.cpp main.cpp
OBJ = ${SRC:.cpp=.o}

.cpp.o:
	${CXX} -c ${CXXFLAGS} -o $*.o $<

seq: ${OBJ}
	${CXX} ${CXXFLAGS} -o seq ${OBJ}

clean:
	rm -rf seq ${OBJ}
