#include "utility.h"

namespace llvm
{
    string Utility::getShortValueName(Value *v)
    {
        if (v->getName().str().length() > 0)
        {
            return "%" + v->getName().str();
        }
        else if (isa<Instruction>(v))
        {
            std::string s = "";
            raw_string_ostream *strm = new raw_string_ostream(s);
            v->print(*strm);
            std::string inst = strm->str();
            size_t idx1 = inst.find("%");
            size_t idx2 = inst.find(" ", idx1);
            if (idx1 != std::string::npos && idx2 != std::string::npos)
            {
                return inst.substr(idx1, idx2 - idx1);
            }
            else
            {
                return "\"" + inst + "\"";
            }
        }
        else if (ConstantInt *cint = dyn_cast<ConstantInt>(v))
        {
            std::string s = "";
            raw_string_ostream *strm = new raw_string_ostream(s);
            cint->getValue().print(*strm, true);
            return strm->str();
        }
        else
        {
            std::string s = "";
            raw_string_ostream *strm = new raw_string_ostream(s);
            v->print(*strm);
            std::string inst = strm->str();
            return "\"" + inst + "\"";
        }
    }

    void Utility::printBitVector(BitVector vector, map<string, int> domain, StringRef label)
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

    void Utility::printStats(vector<string> basicBlockOrder, map<string, BitVector> genMap, map<string, BitVector> killMap, map<string, BitVector> in,
                           map<string, BitVector> out, map<string, int> domain)
    {
        for (auto i = 0; i < basicBlockOrder.size(); ++i)
        {
            string bb = basicBlockOrder[i];
            outs() << "Basic Block: " << bb << "\n";
            printBitVector(genMap[bb], domain, "GEN");
            printBitVector(killMap[bb], domain, "KILL");
            printBitVector(in[bb], domain, "IN");
            printBitVector(out[bb], domain, "OUT");
            outs() << "\n";
        }
    }

    string Utility::getBlockLabel(BasicBlock *B)
    {
        if(!B->getName().empty())
            return B->getName().str();
        string str;
        raw_string_ostream OS(str);
        B->printAsOperand(OS, false);
        return OS.str();
    }
}
