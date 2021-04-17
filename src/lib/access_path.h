#ifndef __ACCESS_PATH_H__
#define __ACCESS_PATH_H__

#include <list>
#include <set>

#include "llvm/IR/Instructions.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/IR/CFG.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/Analysis/AliasAnalysis.h"

namespace llvm
{
    using namespace std;
    class AccessPathOperations
    {
        private:
        map<string, set<Instruction *>> aliases;
        public:
        static list<string> frontier();

    };
}

#endif