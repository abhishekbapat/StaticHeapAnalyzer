// ECE 5984 S21 Assignment 2: available.cpp
// Group: monami@vt.edu, abapat28@vt.edu

////////////////////////////////////////////////////////////////////////////////

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
        enum InstructionCategory
        {
            FunctionCall,
            Assignment,
            Use,
            Alloc,
            Return,
            Other
        };

        class HeapAnalysisFramework : public IterativeFramework<string>
        {
        public:
            using IterativeFramework<string>::IterativeFramework;

        protected:
            BitVector TransferFunction(BitVector gen, BitVector kill, BitVector in)
            {
                BitVector ans = kill.flip();
                ans &= in;  // IN[B] - KILL[B]
                ans |= gen; // GEN U (IN[B] - KILL[B])
                return ans;
            }
        };

    public:
        static char ID;

        HeapAnalysis() : FunctionPass(ID) {}
        ~HeapAnalysis() {}

        bool doInitialization(Module &M) override
        {
            return false;
        }

        bool runOnFunction(Function &F) override
        {
            AliasAnalysis &aa = getAnalysis<AAResultsWrapperPass>().getAAResults();
            map<string, set<string>> dominators = getAnalysis<Dominators>().returnDominators();
            set<Instruction *> pointers;

            for (BasicBlock &BB : F)
            {
                for (Instruction &I : BB)
                {
                    if (I.getType()->isPointerTy())
                        outs() << "Ptrs: " << Utility::getShortValueName(&I) << "\n";
                        // pointers.insert(&I);
                }
            }

            for(auto it = dominators.begin(); it != dominators.end(); ++it)
            {
                string bbName = it->first;
                set<string> doms = it->second;
                outs() << bbName << ": {" << " ";
                for (auto dom = doms.begin(); dom != doms.end(); ++dom)
                {
                    outs() << *dom << " ";
                }
                outs() << "}\n";
            }

            return false;
        }

        void getAnalysisUsage(AnalysisUsage &AU) const
        {
            AU.addRequired<AAResultsWrapperPass>();
            AU.addRequired<Dominators>();
        }
    };

    char HeapAnalysis::ID = 0;
    RegisterPass<HeapAnalysis> X("heap-analysis", "ECE 5984 LICM Pass");
}
