#ifndef __DOMINATORS_H__
#define __DOMINATORS_H__

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Support/raw_ostream.h"

#include "dataflow.h"

using namespace llvm;
using namespace std;

namespace llvm
{
    class Dominators : public FunctionPass
    {
    private:
        Direction _direction;
        MeetOperator _meetOperator;
        map<string, set<string>> dominatorsMap;
        void _printBitVector(BitVector, map<string, int>, StringRef);
        void _generateDominatorsMap(vector<BitVector>, map<string, int>);
    public:
        static char ID;
        Dominators();
        ~Dominators();
        void getAnalysisUsage(AnalysisUsage &AU) const override;
        bool doInitialization(Module &M) override;
        bool runOnFunction(Function &F) override;
        map<string, set<string>> returnDominators();
    };
}

#endif