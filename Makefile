OBJECTS=mysort stdsort threadsort tbbsort steelsort
TBB=/home/ami650/src/gcc-4.6.0/tbb30_20110427oss/

.SILENT:
.PHONY: all clean compile ${OBJECTS}

all: ${OBJECTS}

clean:
	rm -f ${OBJECTS}

compile:
	g++ -O3 -march=native mysort.cpp -o mysort
	g++ -O3 -march=native stdsort.cpp -o stdsort
	g++ -O3 -march=native -std=c++0x threadsort.cpp -o threadsort -pthread
	g++ -O3 -march=native -std=c++0x -I${TBB}/include -L${TBB}/build/linux_intel64_gcc_cc4.6.1_libc2.5_kernel2.6.18_release tbbsort.cpp -o tbbsort -pthread -ltbb
	g++ -O3 -march=native -std=c++0x -DSPLIT=16 steelsort.cpp -o steelsort -pthread

mysort:
	echo Running $@
	/usr/bin/time ./mysort 50000000
	echo

stdsort:
	echo Running $@
	/usr/bin/time ./stdsort
	echo

threadsort:
	echo Running $@
	/usr/bin/time ./threadsort
	echo

tbbsort:
	echo Running $@
	/usr/bin/time ./tbbsort
	echo

steelsort:
	echo Running $@
	/usr/bin/time ./steelsort 50000000
	echo
