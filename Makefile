OBJECTS=mysort gsort stdsort threadsort tbbsort steelsort jonderry ForkJoinQuicksortTask
TBB=/home/ami650/src/gcc-4.6.0/tbb30_20110427oss/
JAVA_HOME=/home/ami650/src/java/jdk1.7.0/

.SILENT:
.PHONY: all clean compile cache ${OBJECTS}

all: cache ${OBJECTS}

clean:
	rm -f ${OBJECTS}

cache:
	cat gaussian.dat > /dev/null

mysort:
	g++ -O3 -march=native mysort.cpp -o mysort
	echo Running $@
	/usr/bin/time ./mysort 50000000
	echo

gsort:
	g++ -O3 -march=native gsort.cpp -o gsort
	echo Running $@
	/usr/bin/time ./gsort 50000000
	echo

stdsort:
	g++ -O3 -march=native stdsort.cpp -o stdsort
	echo Running $@
	/usr/bin/time ./stdsort
	echo

threadsort:
	g++ -O3 -march=native -std=c++0x threadsort.cpp -o threadsort -pthread
	echo Running $@
	/usr/bin/time ./threadsort
	echo

tbbsort:
	g++ -O3 -march=native -std=c++0x -I${TBB}/include -L${TBB}/build/linux_intel64_gcc_cc4.6.1_libc2.5_kernel2.6.18_release tbbsort.cpp -o tbbsort -pthread -ltbb
	echo Running $@
	/usr/bin/time ./tbbsort
	echo

steelsort:
	g++ -O3 -march=native -std=c++0x -DSPLIT=16 steelsort.cpp -o steelsort -pthread
	echo Running $@
	/usr/bin/time ./steelsort 50000000
	echo

jonderry:
	g++ -O3 -march=native jonderry.cpp -o jonderry
	echo Running $@
	/usr/bin/time ./jonderry 50000000
	echo

ForkJoinQuicksortTask:
	${JAVA_HOME}/bin/javac ForkJoinQuicksortTask.java
	echo Running $@
	/usr/bin/time ${JAVA_HOME}/bin/java ForkJoinQuicksortTask 50000000
	echo
