#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/User.h"
#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"

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
    private:
        set<Value *> _mallocPtrs;
        map<string, set<string>> _nonLivePtrs;
        set<BasicBlock *> _blocksInLoop;
        Instruction *_terminatingInstr;

        void _printMap(map<string, set<string>> m)
        {
            for (auto it = m.begin(); it != m.end(); ++it)
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
        }

        void _findMallocPtr(BasicBlock *BB)
        {
            for (Instruction &I : *BB)
            {
                if (I.getType()->isPointerTy())
                {
                    if (isa<CallInst>(&I))
                    {
                        Function *func_name = cast<CallInst>(&I)->getCalledFunction();
                        outs() << "\nPointer name: " << Utility::getShortValueName(&I) << "\n";
                        if (func_name->getName() == "malloc" || func_name->getName() == "_Znmw")
                        {
                            if (_mallocPtrs.find(&I) == _mallocPtrs.end())
                                _mallocPtrs.insert(&I);
                        }
                    }
                }
            }
        }
        void _freePointer(BasicBlock *BB)
        {
            set<Value *> temp;
            for (Value *p : _mallocPtrs)
            {
                outs() << "Freeing pointer: " << Utility::getShortValueName(p) << "\n";
                Instruction *newInst = CallInst::CreateFree(p, BB);
                newInst->insertBefore(_terminatingInstr);
                temp.insert(p);
            }
            for (auto it = temp.begin(); it != temp.end(); ++it)
            {
                Value *p = *it;
                _mallocPtrs.erase(p);
            }
        }

    public:
        static char ID;
        HeapTransform() : FunctionPass(ID) {}
        ~HeapTransform() {}

        void getAnalysisUsage(AnalysisUsage &AU) const override
        {
            AU.setPreservesCFG();
            AU.addRequired<Dominators>();
            AU.addRequired<HeapAnalysis>();
            AU.addRequired<AAResultsWrapperPass>();
            AU.addRequired<LoopInfoWrapperPass>();
        }

        bool doInitialization(Module &M) override
        {
            return false;
        }

        void computeInstrToFree(BasicBlock *B, AliasAnalysis &AA)
        {
            Value *terminatingValue = NULL;
            for (BasicBlock::reverse_iterator i = B->rbegin(); i != B->rend(); ++i)
            {
                Instruction *I = &*i;
                Value *val = I;
                if (I->isTerminator())
                {
                    // if(!isa<ReturnInst>(I))
                    //     continue;
                    _terminatingInstr = I;
                    for (User::op_iterator itr = I->op_begin(); itr != I->op_end(); ++itr)
                    {
                        terminatingValue = *itr;
                        outs() << "\nterminator instruction: " << Utility::getShortValueName(terminatingValue) << "\n";
                    }
                }
                else if (Utility::getShortValueName(val) == Utility::getShortValueName(terminatingValue))
                {
                    for (User::op_iterator itr = I->op_begin(); itr != I->op_end(); ++itr)
                    {
                        Value *userVal = *itr;

                        outs() << "\npointer instruction: " << Utility::getShortValueName(userVal) << "\n";
                        set<Value *> temp;
                        for (Value *p : _mallocPtrs)
                        {
                            bool isAlias = AA.alias(userVal, p);
                            if (isAlias == true)
                            {
                                temp.insert(p);
                                //_mallocPtrs.erase(p);
                                outs() << "alias pointer check: " << Utility::getShortValueName(p) << " is alias\n";
                            }
                        }

                        for (auto it = temp.begin(); it != temp.end(); ++it)
                        {
                            _mallocPtrs.erase(*it);
                        }
                    }
                    break;
                }
            }
        }

        bool runOnFunction(Function &F) override
        {
            map<string, set<string>> dominatorsMap = getAnalysis<Dominators>().returnDominators();
            map<string, set<string>> in = getAnalysis<HeapAnalysis>().returnIn();
            map<string, set<string>> out = getAnalysis<HeapAnalysis>().returnOut();
            AliasAnalysis &AA = getAnalysis<AAResultsWrapperPass>().getAAResults();
            LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

            //Identify malloc pointers in loop
            BasicBlock *latch;
            for (Loop *L : LI)
            {
                for (BasicBlock *BB : L->getBlocks())
                {
                    outs() << "\nBB name in loop: " << Utility::getBlockLabel(BB) << "\n";
                    _blocksInLoop.insert(BB);
                    _findMallocPtr(BB);
                }
                latch = L->getLoopLatch();
                outs() << "Loop latch: " << Utility::getBlockLabel(latch) << "\n";
                computeInstrToFree(latch, AA);
            }
            _freePointer(latch);

            // outs() << "Dominators:\n";
            // _printMap(dominatorsMap);

            // outs() << "INs:\n";
            // _printMap(in);

            // outs() << "OUTs:\n";
            // _printMap(out);

            //Identify malloc pointers in rest of the function
            for (BasicBlock &BB : F)
            {
                if (_blocksInLoop.find(&BB) == _blocksInLoop.end())
                    _findMallocPtr(&BB);
            }

            BasicBlock *lastBB;

            for (po_iterator<BasicBlock *> FI = po_begin(&F.getEntryBlock()); FI != po_end(&F.getEntryBlock()); ++FI)
            {
                if (_blocksInLoop.find(*FI) != _blocksInLoop.end())
                    continue;
                lastBB = *FI;
                outs() << "\nb name: " << Utility::getBlockLabel(lastBB);
                computeInstrToFree(lastBB, AA);
                break;
            }
            // for(auto it = _mallocPtrs.begin(); it != _mallocPtrs.end(); ++it)
            // {
            //     Value *p = *it;
            //     outs() << Utility::getShortValueName(p) << "\n";
            // }
            _freePointer(lastBB);

            return true;
        }
    };

    char HeapTransform::ID = 0;
    static RegisterPass<HeapTransform> X("transform", "Heap Transformation");
}