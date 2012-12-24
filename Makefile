
SOURCES=${wildcard *.cpp}

all: httpserver

httpserver: ${SOURCES:.cpp=.o}
	gcc -g0 -o $@ $^ -lstdc++

%.o: %.cpp
	gcc -MMD -std=c++0x -O2 -g0 -Wall -Wextra -Weffc++ -Werror -c -o $@ $<

clean:
	rm -f *.o *.d httpserver

-include ${SOURCES:.cpp=.d}
