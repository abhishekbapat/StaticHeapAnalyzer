INC=-I/usr/local/include/
all: heap_analysis.so dominators.so transform.so

CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) $(INC) -g -O0 -fPIC

./lib/dataflow.o: ./lib/dataflow.cpp ./lib/dataflow.h
./lib/utility.o: ./lib/utility.cpp ./lib/utility.h
dominators.o: dominators.cpp dominators.h
heap_analysis.o: heap_analysis.cpp heap_analysis.h

dominators.so: dominators.o ./lib/dataflow.o
	$(CXX) -dylib -shared $^ -o $@

heap_analysis.so: heap_analysis.o ./lib/dataflow.o ./lib/utility.o
	$(CXX) -dylib -shared $^ -o $@

transform.so: transform.o ./lib/dataflow.o ./lib/utility.o
	$(CXX) -dylib -shared $^ -o $@

clean:
	rm -f *.o *~ *.so

.PHONY: clean all
