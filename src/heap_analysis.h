#ifndef __HEAP_ANALYSIS_H__
#define __HEAP_ANALYSIS_H__

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/AliasAnalysis.h"

#include "./lib/dataflow.h"
#include "./lib/utility.h"
#include "dominators.h"

using namespace llvm;
using namespace std;

namespace llvm
{
    class HeapAnalysis : public FunctionPass
    {
    private:
        Direction _direction;
        MeetOperator _meetOperator;
        map<string, set<string>> _in;
        map<string, set<string>> _out;
        void _generateOutput(map<string, BitVector>, map<string, BitVector>, map<string, int>);
    public:
        static char ID;
        HeapAnalysis();
        ~HeapAnalysis();
        void getAnalysisUsage(AnalysisUsage &AU) const override;
        bool doInitialization(Module &M) override;
        bool runOnFunction(Function &F) override;
        map<string, set<string>> returnIn();
        map<string, set<string>> returnOut();
    };
}

#endif