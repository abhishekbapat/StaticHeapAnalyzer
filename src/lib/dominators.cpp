#include "dominators.h"

using namespace llvm;

class DominatorFramework : public IterativeFramework<string>
{
public:
    using IterativeFramework<string>::IterativeFramework;

protected:
    BitVector TransferFunction(BitVector gen, BitVector kill, BitVector in)
    {
        BitVector ans = in;
        ans |= gen;
        return ans;
    }
};

void Dominators::_printBitVector(BitVector vector, map<string, int> domain, StringRef label)
{
    outs() << label << ": \n";
    outs() << "{";
    for (auto dom_iter = domain.begin(); dom_iter != domain.end(); ++dom_iter)
    {
        string bb = dom_iter->first;
        if (vector[domain[bb]] == true)
            outs() << bb << " ";
    }
    outs() << "}\n";
}

void Dominators::_generateDominatorsMap(vector<BitVector> vector, map<string, int> domain)
{
    for (auto dom = domain.begin(); dom != domain.end(); ++dom)
    {
        string block = dom->first;
        set<string> outputs;
        for (auto dom_iter = domain.begin(); dom_iter != domain.end(); ++dom_iter)
        {
            string bb = dom_iter->first;
            if (vector[dom->second][domain[bb]] == true)
                outputs.insert(bb);
        }
        this->dominatorsMap[block] = outputs;
    }
}

Dominators::Dominators() : FunctionPass(ID) {}
Dominators::~Dominators() {}

void Dominators::getAnalysisUsage(AnalysisUsage &AU) const
{
    AU.setPreservesAll();
}

bool Dominators::doInitialization(Module &M)
{
    _meetOperator = Intersection;
    _direction = Forward;
    return false;
}

bool Dominators::runOnFunction(Function &F)
{
    map<string, int> domain;
    map<string, BitVector> genMap, killMap;
    int basicBlockCounter = 0;
    ReversePostOrderTraversal<Function *> rpo(&F);
    for (ReversePostOrderTraversal<Function *>::rpo_iterator i = rpo.begin(); i != rpo.end(); ++i)
    {
        BasicBlock *block = *i;
        domain[block->getName().str()] = basicBlockCounter;
        basicBlockCounter++;
    }
    BitVector boundary(basicBlockCounter, false);
    for (auto i = domain.begin(); i != domain.end(); ++i)
    {
        string bb = i->first;
        killMap[bb] = boundary;
        BitVector gen(basicBlockCounter, false);
        gen[i->second] = true;
        genMap[bb] = gen;
    }

    DominatorFramework framework(domain, _direction, _meetOperator, boundary, basicBlockCounter, basicBlockCounter, genVector, killVector);
    framework.Init(F);
    framework.Compute();
    vector<BitVector> in = framework.ReturnInitial();
    vector<BitVector> out = framework.ReturnFinal();
    // outs() << "Function: " << F.getName() << "\n\n";
    // for (auto bb_iter = domain.begin(); bb_iter != domain.end(); ++bb_iter)
    // {
    //     outs() << "Basic Block name: " << bb_iter->first << "\n";
    //     _printBitVector(in[bb_iter->second], domain, "IN[BB]");
    //     _printBitVector(out[bb_iter->second], domain, "OUT[BB]");
    //     outs() << "\n";
    // }
    _generateDominatorsMap(out, domain);
    // Did not modify the incoming Function.
    return false;
}

map<string, set<string>> Dominators::returnDominators()
{
    return this->dominatorsMap;
}

// LLVM uses the address of this static member to identify the pass, so the
// initialization value is unimportant.
char Dominators::ID = 0;
static RegisterPass<Dominators> X("dominators", "5984: Function Information");