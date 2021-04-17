INC=-I/usr/local/include/
all: heap_analysis.so

CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) $(INC) -g -O0 -fPIC

./lib/dataflow.o: ./lib/dataflow.cpp ./lib/dataflow.h
./lib/utility.o: ./lib/utility.cpp ./lib/utility.h

heap_analysis.so: heap_analysis.o ./lib/dataflow.o ./lib/utility.o
	$(CXX) -dylib -shared $^ -o $@

clean:
	rm -f *.o *~ *.so

.PHONY: clean all
