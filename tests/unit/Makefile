INC=-I/usr/local/include/
all: test.so

CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) $(INC) -g -O0 -fPIC

../../src/lib/dataflow.o: ../../src/lib/dataflow.cpp ../../src/lib/dataflow.h

test.so: test.o ../../src/lib/dataflow.o
	$(CXX) -dylib -shared $^ -o $@

clean:
	rm -f *.o *~ *.so

.PHONY: clean all
