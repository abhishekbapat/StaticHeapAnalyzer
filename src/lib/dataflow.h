/*
 * Copyright (c) 2021 Abhishek Bapat.  All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef __CLASSICAL_DATAFLOW_H__
#define __CLASSICAL_DATAFLOW_H__

#include <vector>
#include <set>
#include <map>

#include "llvm/IR/Instructions.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/IR/CFG.h"
#include "llvm/ADT/PostOrderIterator.h"

namespace llvm
{
    using namespace std;

    enum Direction
    {
        Forward,
        Backward
    };

    enum MeetOperator
    {
        Intersection,
        Union,
    };

    // A context free implementation of the classical iterative dataflow framework.
    template <typename T>
    class IterativeFramework
    {
    private:
        bool _initialized;
        Direction _direction;
        MeetOperator _meetOperator;
        map<T, int> _domain;                     // Maps domain set values with an index representing the bitvector position of the set value.
        BitVector _boundary;                     // Boundary condition.
        map<string, BitVector> _initial;         // Values of IN[BB]/OUT[BB] mapped to the BB name.
        map<string, BitVector> _final;           // Values of OUT[BB]/IN[BB] mapped to the BB name.
        map<string, BitVector> _gen;             // Gen bitvectors of every BB mapped to the BB name.
        map<string, BitVector> _kill;            // Kill bitvectors of every BB mapped to the BB name.
        map<string, set<string>> _basicBlockMap; // To keep track of the successors/predecessors of each basic block.
        vector<string> _basicBlockOrder;         // The order in which basic blocks will be executed in compute.
        int _numBasicBlocks;                     // Total number of basic blocks.
        int _numElements;                        // Total number of elements in the domain.
        bool _hasChanged(map<string, BitVector>, map<string, BitVector>);
        void _updateInitialAndFinal(map<string, BitVector>, map<string, BitVector>);

    protected:
        virtual BitVector TransferFunction(BitVector gen, BitVector kill, BitVector in) = 0;

    public:
        IterativeFramework(map<T, int>, Direction, MeetOperator, BitVector, int, int, map<string, BitVector>, map<string, BitVector>,
                           map<string, set<string>>, vector<string>);
        ~IterativeFramework();
        void Init();
        void Compute();
        map<string, BitVector> ReturnFinal();
        map<string, BitVector> ReturnInitial();
    };
}

#endif
