// ECE 5984 S21 Assignment 2: available.cpp
// Group: monami@vt.edu, abapat28@vt.edu

////////////////////////////////////////////////////////////////////////////////

#include "heap_analysis.h"

using namespace llvm;
using namespace std;

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

void HeapAnalysis::_generateOutput(map<string, BitVector> in, map<string, BitVector> out, map<string, int> domain)
{
    for (auto it = in.begin(); it != in.end(); ++it)
    {
        string bb = it->first;
        set<string> ins;
        for(auto dom = domain.begin(); dom != domain.end(); ++dom)
        {
            string ptr = dom->first;
            BitVector vec = it->second;
            if(vec[domain[ptr]] == true)
                ins.insert(ptr);
        }
        this->_in[bb] = ins;
    }
    for (auto it = out.begin(); it != out.end(); ++it)
    {
        string bb = it->first;
        set<string> outs;
        for(auto dom = domain.begin(); dom != domain.end(); ++dom)
        {
            string ptr = dom->first;
            BitVector vec = it->second;
            if(vec[domain[ptr]] == true)
                outs.insert(ptr);
        }
        this->_out[bb] = outs;
    }
}

HeapAnalysis::HeapAnalysis() : FunctionPass(ID) {}
HeapAnalysis::~HeapAnalysis() {}

bool HeapAnalysis::doInitialization(Module &M)
{
    return false;
}

bool HeapAnalysis::runOnFunction(Function &F)
{
    set<Instruction *> pointers;
    int elementCounter = 0;
    int basicBlockCounter = 0;
    map<string, int> domain;
    vector<string> basicBlockOrder;
    map<string, BitVector> lDirect;
    map<string, BitVector> lKill;
    map<string, set<string>> basicBlockMap;

    for (BasicBlock &BB : F)
    {
        string bbName = BB.getName().str();
        for (Instruction &I : BB)
        {
            if (I.getType()->isPointerTy())
            {
                string instr = Utility::getShortValueName(&I);
                domain[instr] = elementCounter;
                elementCounter++;
            }
        }
        set<string> succs;
        for (auto it = succ_begin(&BB); it != succ_end(&BB); ++it)
        {
            BasicBlock *succBlock = *it;
            string succName = succBlock->getName().str();
            succs.insert(succName);
        }
        basicBlockMap[bbName] = succs;
        basicBlockCounter++;
    }

    for (BasicBlock &BB : F)
    {
        BitVector direct(elementCounter, false);
        BitVector kill(elementCounter, false);
        string bbName = BB.getName().str();
        lDirect[bbName] = direct;
        lKill[bbName] = kill;
    }

    BitVector boundary(elementCounter, false);
    for (po_iterator<BasicBlock *> it = po_begin(&F.getEntryBlock()); it != po_end(&F.getEntryBlock()); ++it)
    {
        BasicBlock *bb = *it;
        string bbName = bb->getName().str();
        basicBlockOrder.push_back(bbName);
        for (auto rit = bb->rbegin(); rit != bb->rend(); ++rit)
        {
            Instruction *i = &*rit;
            for (auto opNum = 0; opNum < i->getNumOperands(); opNum++)
            {
                string op = Utility::getShortValueName(i->getOperand(opNum));
                if (domain.find(op) != domain.end()) // operator exists in domain.
                {
                    int index = domain[op];
                    lDirect[bbName].set(index);
                }
            }
            string instr = Utility::getShortValueName(i);
            if (domain.find(instr) != domain.end()) // lhs exists in domain.
            {
                int index = domain[instr];
                lKill[bbName].set(index);
            }
        }
    }

    HeapAnalysisFramework framework(domain, Backward, Union, boundary, basicBlockCounter, elementCounter, lDirect, lKill, basicBlockMap, basicBlockOrder);
    framework.Init();
    framework.Compute();
    map<string, BitVector> out = framework.ReturnInitial();
    map<string, BitVector> in = framework.ReturnFinal();

    // Utility::printStats(basicBlockOrder, lDirect, lKill, in, out, domain);

    _generateOutput(in, out, domain);

    return false;
}

map<string, set<string>> HeapAnalysis::returnIn()
{
    return this->_in;
}

map<string, set<string>> HeapAnalysis::returnOut()
{
    return this->_out;
}

void HeapAnalysis::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.setPreservesAll();
}

char HeapAnalysis::ID = 0;
RegisterPass<HeapAnalysis> X("heap-analysis", "Heap Analysis Pass");
