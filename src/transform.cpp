#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/AliasAnalysis.h"

#include "./lib/utility.h"
#include "dominators.h"
#include "heap_analysis.h"

using namespace llvm;
using namespace std;

/*
 * To run this pass navigate to src and run make.
 * Then run: opt -load=./dominators.so -load=./heap_analysis.so -load=./transform.so -transform ../tests/benchmarks/test4-m2r.bc
 */

namespace llvm
{
    class HeapTransform : public FunctionPass
    {
    public:
        static char ID;
        HeapTransform() : FunctionPass(ID) {}
        ~HeapTransform() {}

        void getAnalysisUsage(AnalysisUsage &AU) const override
        {
            AU.setPreservesCFG();
            AU.addRequired<Dominators>();
            AU.addRequired<HeapAnalysis>();
        }

        bool doInitialization(Module &M) override
        {
            return false;
        }

        bool runOnFunction(Function &F) override
        {
            map<string, set<string>> dominatorsMap = getAnalysis<Dominators>().returnDominators();
            map<string, set<string>> in = getAnalysis<HeapAnalysis>().returnIn();
            map<string, set<string>> out = getAnalysis<HeapAnalysis>().returnOut();

            outs() << "Dominators:\n";
            for (auto it = dominatorsMap.begin(); it != dominatorsMap.end(); ++it)
            {
                string bbName = it->first;
                set<string> doms = it->second;
                outs() << bbName << ": {"
                       << " ";
                for (auto dom = doms.begin(); dom != doms.end(); ++dom)
                {
                    outs() << *dom << " ";
                }
                outs() << "}\n";
            }
            outs() << "\n";

            outs() << "INs:\n";
            for (auto it = in.begin(); it != in.end(); ++it)
            {
                string bbName = it->first;
                set<string> ins = it->second;
                outs() << bbName << ": {"
                       << " ";
                for (auto i = ins.begin(); i != ins.end(); ++i)
                {
                    outs() << *i << " ";
                }
                outs() << "}\n";
            }
            outs() << "\n";

            outs() << "OUTs:\n";
            for (auto it = out.begin(); it != out.end(); ++it)
            {
                string bbName = it->first;
                set<string> outputs = it->second;
                outs() << bbName << ": {"
                       << " ";
                for (auto o = outputs.begin(); o != outputs.end(); ++o)
                {
                    outs() << *o << " ";
                }
                outs() << "}\n";
            }
            outs() << "\n";

            return false;
        }
    };

    char HeapTransform::ID = 0;
    static RegisterPass<HeapTransform> X("transform", "Heap Transformation");
}