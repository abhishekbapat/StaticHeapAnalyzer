#ifndef __UTILITY_H
#define __UTILITY_H

#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using namespace std;

namespace llvm
{
    class Utility
    {
    public:
        static string getShortValueName(Value *v);
    };
}
#endif