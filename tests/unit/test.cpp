// ECE 5984 test.cpp
// Group: abapat28@vt.edu, monami@vt.edu

////////////////////////////////////////////////////////////////////////////////
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "../../src/lib/dataflow.h"

using namespace llvm;
using namespace std;

namespace
{
    class DataflowTests : public FunctionPass
    {
    private:
        class Framework : public IterativeFramework<string>
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

        void _printBitVector(BitVector vector, map<string, int> domain, StringRef label)
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

        void _printStats(vector<string> basicBlockOrder, map<string, BitVector> genVector, map<string, BitVector> killVector, map<string, BitVector> in,
                         map<string, BitVector> out, map<string, int> domain)
        {
            for (auto i = 0; i < basicBlockOrder.size(); ++i)
            {
                string bb = basicBlockOrder[i];
                outs() << "Basic Block " << bb << ":\n";
                _printBitVector(genVector[bb], domain, "GEN");
                _printBitVector(killVector[bb], domain, "KILL");
                _printBitVector(in[bb], domain, "IN");
                _printBitVector(out[bb], domain, "OUT");
                outs() <<"\n";
            }
        }

        //forward, union
        void _test1()
        {
            Direction _direction = Forward;
            MeetOperator _meetOperator = Union;
            int basicBlockCounter = 4;
            int domainCounter = 7;
            BitVector boundary(domainCounter, false);

            //GEN VECTORS
            BitVector gen1 = boundary;
            BitVector gen2 = boundary;
            BitVector gen3 = boundary;
            BitVector gen4 = boundary;
            gen1.set(0, 3);
            gen2.set(3, 5);
            gen3.set(5);
            gen4.set(6);

            //kill VECTORS
            BitVector kill1 = boundary;
            BitVector kill2 = boundary;
            BitVector kill3 = boundary;
            BitVector kill4 = boundary;
            kill1.set(3, 7);
            kill2.set(0, 2);
            kill2.set(6);
            kill3.set(2);
            kill4.set(0);
            kill4.set(3);

            //basic block map
            set<string> b1 = {};
            set<string> b2 = {"b1", "b4"};
            set<string> b3 = {"b2"};
            set<string> b4 = {"b2", "b3"};

            map<string, set<string>> basicBlockMap = {{"b1", b1}, {"b2", b2}, {"b3", b3}, {"b4", b4}};
            vector<string> basicBlockOrder = {"b1", "b2", "b3", "b4"};

            map<string, int> domain = {{"d1", 0}, {"d2", 1}, {"d3", 2}, {"d4", 3}, {"d5", 4}, {"d6", 5}, {"d7", 6}};
            map<string, BitVector> genVector = {{"b1", gen1}, {"b2", gen2}, {"b3", gen3}, {"b4", gen4}};

            map<string, BitVector> killVector = {{"b1", kill1}, {"b2", kill2}, {"b3", kill3}, {"b4", kill4}};
            Framework framework(domain, _direction, _meetOperator, boundary, basicBlockCounter, domainCounter, genVector, killVector, basicBlockMap, basicBlockOrder);
            framework.Init();
            framework.Compute();
            map<string, BitVector> in = framework.ReturnInitial();
            map<string, BitVector> out = framework.ReturnFinal();

            BitVector out_computed = out["b4"];
            BitVector out_expected = boundary;
            out_expected.set(4, 7);
            out_expected.set(2);

            // _printStats(basicBlockOrder, genVector, killVector, in, out, domain);

            outs() << "Test 1: ";
            if (out_computed == out_expected)
                outs() << "Passed \n\n";
            else
                outs() << "Failed \n\n";
        }

        //forward, intersection
        void _test2()
        {
            Direction _direction = Forward;
            MeetOperator _meetOperator = Intersection;
            int basicBlockCounter = 4;
            int domainCounter = 7;
            BitVector boundary(domainCounter, false);

            //GEN VECTORS
            BitVector gen1 = boundary;
            BitVector gen2 = boundary;
            BitVector gen3 = boundary;
            BitVector gen4 = boundary;
            gen1.set(0, 3);
            gen2.set(3, 5);
            gen3.set(5);
            gen4.set(6);

            //kill VECTORS
            BitVector kill1 = boundary;
            BitVector kill2 = boundary;
            BitVector kill3 = boundary;
            BitVector kill4 = boundary;
            kill1.set(3, 7);
            kill2.set(0, 2);
            kill2.set(6);
            kill3.set(2);
            kill4.set(0);
            kill4.set(3);

            //basic block map
            set<string> b1 = {};
            set<string> b2 = {"b1", "b4"};
            set<string> b3 = {"b2"};
            set<string> b4 = {"b2", "b3"};

            map<string, set<string>> basicBlockMap = {{"b1", b1}, {"b2", b2}, {"b3", b3}, {"b4", b4}};
            vector<string> basicBlockOrder = {"b1", "b2", "b3", "b4"};

            map<string, int> domain = {{"d1", 0}, {"d2", 1}, {"d3", 2}, {"d4", 3}, {"d5", 4}, {"d6", 5}, {"d7", 6}};
            map<string, BitVector> genVector = {{"b1", gen1}, {"b2", gen2}, {"b3", gen3}, {"b4", gen4}};

            map<string, BitVector> killVector = {{"b1", kill1}, {"b2", kill2}, {"b3", kill3}, {"b4", kill4}};
            Framework framework(domain, _direction, _meetOperator, boundary, basicBlockCounter, domainCounter, genVector, killVector, basicBlockMap, basicBlockOrder);
            framework.Init();
            framework.Compute();
            map<string, BitVector> in = framework.ReturnInitial();
            map<string, BitVector> out = framework.ReturnFinal();

            BitVector out_computed = out["b4"];
            BitVector out_expected = boundary;
            out_expected.set(4);
            out_expected.set(6);

            // _printStats(basicBlockOrder, genVector, killVector, in, out, domain);

            outs() << "Test 2: ";
            if (out_computed == out_expected)
                outs() << "Passed \n\n";
            else
                outs() << "Failed \n\n";
        }

        //backward, union
        void _test3()
        {
            Direction _direction = Backward;
            MeetOperator _meetOperator = Union;
            int basicBlockCounter = 4;
            int domainCounter = 7;
            BitVector boundary(domainCounter, false);

            //GEN VECTORS
            BitVector gen1 = boundary;
            BitVector gen2 = boundary;
            BitVector gen3 = boundary;
            BitVector gen4 = boundary;
            gen1.set(0, 3);
            gen2.set(3, 5);
            gen3.set(5);
            gen4.set(6);

            //kill VECTORS
            BitVector kill1 = boundary;
            BitVector kill2 = boundary;
            BitVector kill3 = boundary;
            BitVector kill4 = boundary;
            kill1.set(3, 7);
            kill2.set(0, 2);
            kill2.set(6);
            kill3.set(2);
            kill4.set(0);
            kill4.set(3);

            //basic block map
            set<string> b1 = {"b2"};
            set<string> b2 = {"b3", "b4"};
            set<string> b3 = {"b4"};
            set<string> b4 = {"b2"};

            map<string, set<string>> basicBlockMap = {{"b1", b1}, {"b2", b2}, {"b3", b3}, {"b4", b4}};
            vector<string> basicBlockOrder = {"b4", "b3", "b2", "b1"};

            map<string, int> domain = {{"d1", 0}, {"d2", 1}, {"d3", 2}, {"d4", 3}, {"d5", 4}, {"d6", 5}, {"d7", 6}};
            map<string, BitVector> genVector = {{"b1", gen1}, {"b2", gen2}, {"b3", gen3}, {"b4", gen4}};

            map<string, BitVector> killVector = {{"b1", kill1}, {"b2", kill2}, {"b3", kill3}, {"b4", kill4}};
            Framework framework(domain, _direction, _meetOperator, boundary, basicBlockCounter, domainCounter, genVector, killVector, basicBlockMap, basicBlockOrder);
            framework.Init();
            framework.Compute();
            map<string, BitVector> out = framework.ReturnInitial();
            map<string, BitVector> in = framework.ReturnFinal();

            BitVector out_computed = in["b1"];
            BitVector out_expected = boundary;
            out_expected.set(0, 3);

            // _printStats(basicBlockOrder, genVector, killVector, in, out, domain);

            outs() << "Test 3: ";
            if (out_computed == out_expected)
                outs() << "Passed \n\n";
            else
                outs() << "Failed \n\n";
        }

        //backward, intersection
        void _test4()
        {
            Direction _direction = Backward;
            MeetOperator _meetOperator = Intersection;
            int basicBlockCounter = 4;
            int domainCounter = 7;
            BitVector boundary(domainCounter, false);

            //GEN VECTORS
            BitVector gen1 = boundary;
            BitVector gen2 = boundary;
            BitVector gen3 = boundary;
            BitVector gen4 = boundary;
            gen1.set(0, 3);
            gen2.set(3, 5);
            gen3.set(5);
            gen4.set(6);

            //kill VECTORS
            BitVector kill1 = boundary;
            BitVector kill2 = boundary;
            BitVector kill3 = boundary;
            BitVector kill4 = boundary;
            kill1.set(3, 7);
            kill2.set(0, 2);
            kill2.set(6);
            kill3.set(2);
            kill4.set(0);
            kill4.set(3);

            //basic block map
            set<string> b1 = {"b2"};
            set<string> b2 = {"b3", "b4"};
            set<string> b3 = {"b4"};
            set<string> b4 = {"b2"};

            map<string, set<string>> basicBlockMap = {{"b1", b1}, {"b2", b2}, {"b3", b3}, {"b4", b4}};
            vector<string> basicBlockOrder = {"b4", "b3", "b2", "b1"};

            map<string, int> domain = {{"d1", 0}, {"d2", 1}, {"d3", 2}, {"d4", 3}, {"d5", 4}, {"d6", 5}, {"d7", 6}};
            map<string, BitVector> genVector = {{"b1", gen1}, {"b2", gen2}, {"b3", gen3}, {"b4", gen4}};

            map<string, BitVector> killVector = {{"b1", kill1}, {"b2", kill2}, {"b3", kill3}, {"b4", kill4}};
            Framework framework(domain, _direction, _meetOperator, boundary, basicBlockCounter, domainCounter, genVector, killVector, basicBlockMap, basicBlockOrder);
            framework.Init();
            framework.Compute();
            map<string, BitVector> out = framework.ReturnInitial();
            map<string, BitVector> in = framework.ReturnFinal();

            BitVector out_computed = in["b1"];
            BitVector out_expected = boundary;
            out_expected.set(0, 3);

            // _printStats(basicBlockOrder, genVector, killVector, in, out, domain);

            outs() << "Test 4: ";
            if (out_computed == out_expected)
                outs() << "Passed \n\n";
            else
                outs() << "Failed \n\n";
        }

    public:
        static char ID;
        DataflowTests() : FunctionPass(ID) {}
        bool doInitialization(Module &M) override
        {
            return false;
        }
        bool runOnFunction(Function &F) override
        {
            _test1();
            _test2();
            _test3();
            _test4();
            return false;
        }
    };

    char DataflowTests::ID = 0;
    RegisterPass<DataflowTests> X("df-tests", "Dataflow framework tests");
}