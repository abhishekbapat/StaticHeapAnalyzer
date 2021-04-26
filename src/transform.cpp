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
        set<Value *> _mallocPtrs, _freePtrs;
        map<Value *, string> _mallocPtrBBmap;
        set<BasicBlock *> _blocksInLoop;
        Instruction *_terminatingInstr;
        map<string, Value *> _domain;

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
                        outs() << "Malloc pointer name: " << Utility::getShortValueName(&I) << "\n";
                        if (func_name->getName() == "malloc" || func_name->getName() == "_Znmw")
                        {
                            if (_mallocPtrs.find(&I) == _mallocPtrs.end())
                            {
                                _mallocPtrs.insert(&I);
                                _mallocPtrBBmap[&I] = Utility::getBlockLabel(BB);
                            }
                        }
                    }
                }
            }
        }

        /**
         * 
         * Keep a map of all pointers with key = pointer name (string) and 
         * value = pointer value (Value *)
         *  
         **/
        void _mapStringAndValue(BasicBlock *BB)
        {
            for (Instruction &I : *BB)
            {
                if (I.getType()->isPointerTy())
                {
                    string instr = Utility::getShortValueName(&I);
                    _domain[instr] = &I;
                }
                if (isa<CallInst>(I))
                    continue;
                for (User::op_iterator itr = I.op_begin(); itr != I.op_end(); ++itr)
                {
                    Value *userVal = *itr;
                    string instr = Utility::getShortValueName(userVal);
                    if (userVal->getType()->isPointerTy())
                    {
                        _domain[instr] = userVal;
                    }
                }
            }
        }

        /**
         * 
         * Compute pointers to free in all blocks 
         * check if IN values are present in OUT values => yes, do nothing : no, add to _freePtrs to be freed later
         *  
         **/
        void _findInstrToFree(BasicBlock *B, AliasAnalysis &AA, map<string, set<string>> in, map<string, set<string>> out)
        {
            string BBname = Utility::getBlockLabel(B);
            set<string> inPtrs = in[BBname];
            set<string> outPtrs = out[BBname];

            for (auto it = inPtrs.begin(); it != inPtrs.end(); ++it)
            {
                if (outPtrs.find(*it) == outPtrs.end())
                {
                    Value *p = _domain[*it];
                    if (p == NULL)
                        continue;
                    for (Value *val : _mallocPtrs)
                    {
                        bool isAlias = AA.alias(val, p);
                        if (isAlias == true)
                            _freePtrs.insert(val);
                    }
                }
            }
        }

        /**
         * 
         * Compute pointers to free in the "last" block OR,
         * the block which has out values as {} "empty"
         * 
         * We need to check here whether return statement uses a pointer.
         *  
         **/
        void _computeInstrToFree(BasicBlock *B, AliasAnalysis &AA)
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
                        outs() << "terminating instruction name: " << Utility::getShortValueName(terminatingValue) << "\n";
                    }
                }
                else if (Utility::getShortValueName(val) == Utility::getShortValueName(terminatingValue))
                {

                    for (User::op_iterator itr = I->op_begin(); itr != I->op_end(); ++itr)
                    {
                        Value *userVal = *itr;

                        outs() << "pointer name in return statement: " << Utility::getShortValueName(userVal) << "\n";
                        set<Value *> temp;
                        for (Value *p : _mallocPtrs)
                        {
                            bool isAlias = AA.alias(userVal, p);
                            if (isAlias == true)
                            {
                                _freePtrs.erase(p);
                                outs() << "alias pointer check: " << Utility::getShortValueName(p) << " is alias\n";
                            }
                            else
                                _freePtrs.insert(p);
                        }
                    }
                    break;
                }
            }
        }

        /**
         * 
         * Check if the instruction to be freed is dominated by the block in which it was created. 
         * Basically, check if the malloc() statements dominate the free() statements
         *  
         **/
        bool _checkDominating(Value *p, map<string, set<string>> domMap, string BBFree)
        {
            string BBname = _mallocPtrBBmap[p];
            set<string> domMapVal = domMap[BBFree];
            if (domMapVal.find(BBname) != domMapVal.end())
                return true;
            return false;
        }
        /**
         * 
         * Add free() instructions
         * Before freeing, check if it is dominated by its malloc() statement
         *  
         **/
        void _freePointer(BasicBlock *BB, map<string, set<string>> dominatorsMap)
        {
            set<Value *> temp;
            for (Value *p : _freePtrs)
            {

                bool isDominating = _checkDominating(p, dominatorsMap, Utility::getBlockLabel(BB));
                if (!isDominating)
                    continue;
                outs() << "Freeing pointer: " << Utility::getShortValueName(p) << "\n";
                Instruction *newInst = CallInst::CreateFree(p, BB);
                newInst->insertBefore(_terminatingInstr);
            }
            _freePtrs.clear();
            _mallocPtrs.clear();
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

        bool runOnFunction(Function &F) override
        {
            map<string, set<string>> dominatorsMap = getAnalysis<Dominators>().returnDominators();
            map<string, set<string>> in = getAnalysis<HeapAnalysis>().returnIn();
            map<string, set<string>> out = getAnalysis<HeapAnalysis>().returnOut();
            AliasAnalysis &AA = getAnalysis<AAResultsWrapperPass>().getAAResults();
            LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

            outs() << "Dominators:\n";
            _printMap(dominatorsMap);

            outs() << "INs:\n";
            _printMap(in);

            outs() << "OUTs:\n";
            _printMap(out);

            /**
            * 
            * Compute pointers to free inside the loop first, IF a loop exists
            *  
            **/
            BasicBlock *latch;
            for (Loop *L : LI)
            {
                outs() << "Inside Loop.\n";
                for (BasicBlock *BB : L->getBlocks())
                {
                    _blocksInLoop.insert(BB);
                    _findMallocPtr(BB);
                    _mapStringAndValue(BB);
                    _findInstrToFree(BB, AA, in, out);
                }

                latch = L->getLoopLatch();
                outs() << "Loop latch name: " << Utility::getBlockLabel(latch) << "\n";
                _computeInstrToFree(latch, AA);
            }

            _freePointer(latch, dominatorsMap);

            /**
            * 
            * Compute pointers to free in the rest of the function
            *  
            **/
            outs() << "\nOutside Loop.\n";
            for (BasicBlock &BB : F)
            {
                if (_blocksInLoop.find(&BB) == _blocksInLoop.end())
                {
                    _findMallocPtr(&BB);
                    _mapStringAndValue(&BB);
                    _findInstrToFree(&BB, AA, in, out);
                }
            }

            BasicBlock *lastBB;

            for (po_iterator<BasicBlock *> FI = po_begin(&F.getEntryBlock()); FI != po_end(&F.getEntryBlock()); ++FI)
            {
                if (_blocksInLoop.find(*FI) != _blocksInLoop.end())
                    continue;
                lastBB = *FI;
                outs() << "Block name to insert free(): " << Utility::getBlockLabel(lastBB) << "\n";
                _computeInstrToFree(lastBB, AA);
                break;
            }

            _freePointer(lastBB, dominatorsMap);

            return true;
        }
    };

    char HeapTransform::ID = 0;
    static RegisterPass<HeapTransform> X("transform", "Heap Transformation");
}