OBJECTS=mysort stdsort threadsort

.SILENT:
.PHONY: all clean ${OBJECTS}

all: ${OBJECTS}

clean:
	rm -f ${OBJECTS}

mysort:
	g++ -O3 -march=native mysort.cpp -o mysort
	echo Running $@
	/usr/bin/time ./mysort 50000000
	echo

threadsort:
	g++ -O3 -march=native -std=c++0x threadsort.cpp -o threadsort -pthread
	echo Running $@
	/usr/bin/time ./threadsort
	echo

stdsort:
	g++ -O3 -march=native stdsort.cpp -o stdsort
	echo Running $@
	/usr/bin/time ./stdsort
	echo
