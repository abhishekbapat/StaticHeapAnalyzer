# StaticHeapAnalyzer

## Group:
- monami@vt.edu
- abapat28@vt.edu

## Directory Structure
```
.
├── README.md
├── src
│   ├── dominators.cpp
│   ├── dominators.h
│   ├── heap_analysis.cpp
│   ├── heap_analysis.h
│   ├── lib
│   │   ├── access_path.h
│   │   ├── dataflow.cpp
│   │   ├── dataflow.h
│   │   ├── utility.cpp
│   │   ├── utility.h
│   ├── Makefile
│   ├── transform.cpp
└── tests
    ├── benchmarks
    │   ├── run-analysis.sh
    │   ├── test1.c
    │   ├── test2.c
    │   ├── test3.c
    │   ├── test4.c
    │   └── view-cfg.sh
    └── unit
        ├── Makefile
        ├── t.bc
        ├── test.cpp
```

## Description
This project is intraprocedural heap analyser implemented in LLVM. The project computes the liveness of heap data and heap data that is not live at the exit of the function. There are a total of three passes implemented, namely:
- Dominators
- HeapAnalysis
- HeapTransform

These passes instantiate the implemented dataflow analysis framework to compute the IN and OUT values of each basic block. The `HeapTransform` pass takes in data from the `Dominators` and the `HeapAnalysis` pass to compute the heap memory that can be freed and frees it. The project also internally uses LLVM's `AliasAnalysis` pass.

### Dominators Pass
This pass computes the dominators of each basic block. This information is needed by the the HeapTranform pass to compute which block can be safely used to free heap memory. The block that contains the `free` function should be dominated by the basic block containing the `malloc` function.

### HeapAnalysis Pass
This pass computes pointers to heap data that are live at the IN and OUT of each basic block.

### HeapTransformation Pass
This pass utlizes data from Dominators and HeapAnalysis to compute the pointers that can be freed and also the location where the free function can be safely inserted.

## Building the Passes
- Navigate to the `src` directory and run the `make` command. This will build all the passes.

## Running the Passes
Four microbenchmarks are written to test these passes inside `./tests/benchmarks`. To run the passes navigate to the `benchmarks` directory and run the command `sudo bash run-analysis.sh testName`. Eg. `sudo bash run-analysis.sh test1`. This will first analyse the bitcode without heap transformation, then run the transformation pass on the bitcode and finally analyse the bitcode again with transformation.