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

#include "dataflow.h"

namespace llvm
{
    template class IterativeFramework<string>;

    template <typename T>
    bool IterativeFramework<T>::_hasChanged(map<string, BitVector> tempInitial, map<string, BitVector> tempFinal)
    {
        if(tempInitial.size() != this->_initial.size())
            return true;
        if(tempFinal.size() != this->_final.size())
            return true;
        for(auto it = tempInitial.begin(); it != tempInitial.end(); ++it)
        {
            string bb = it->first;
            if(this->_initial.find(bb) == this->_initial.end())
                return true;
            if(tempInitial[bb] != this->_initial[bb])
                return true;
        }
        for(auto it = tempFinal.begin(); it != tempFinal.end(); ++it)
        {
            string bb = it->first;
            if(this->_final.find(bb) == this->_final.end())
                return true;
            if(tempFinal[bb] != this->_final[bb])
                return true;
        }
        return false;
    }

    template <typename T>
    void IterativeFramework<T>::_updateInitialAndFinal(map<string, BitVector> tempInitial, map<string, BitVector> tempFinal)
    {
        this->_initial = tempInitial;
        this->_final = tempFinal;
    }

    template <typename T>
    IterativeFramework<T>::IterativeFramework(map<T, int> domain, Direction direction, MeetOperator meetOperator, BitVector boundary,
                                              int numBasicBlocks, int numElements, map<string, BitVector> gen, map<string, BitVector> kill,
                                              map<string, set<string>> basicBlockMap, vector<string> basicBlockOrder)
    {
        this->_domain = domain;
        this->_direction = direction;
        this->_meetOperator = meetOperator;
        this->_boundary = boundary;
        this->_numBasicBlocks = numBasicBlocks;
        this->_numElements = numElements;
        this->_gen = gen;
        this->_kill = kill;
        this->_basicBlockMap = basicBlockMap;
        this->_basicBlockOrder = basicBlockOrder;
        this->_initialized = false;
    }

    template <typename T>
    IterativeFramework<T>::~IterativeFramework() {}

    template <typename T>
    void IterativeFramework<T>::Init() // A bunch of sanity checks and some initializations.
    {
        assert(this->_numBasicBlocks == this->_basicBlockOrder.size());
        assert(this->_numElements == this->_domain.size());
        assert(this->_numBasicBlocks == this->_gen.size());
        assert(this->_numBasicBlocks == this->_kill.size());
        assert(this->_meetOperator == Intersection || this->_meetOperator == Union);
        if (this->_meetOperator == Intersection)
        {
            BitVector finalInit(this->_numElements, true);
            for (auto it = this->_gen.begin(); it != this->_gen.end(); ++it)
            {
                string bb = it->first;
                this->_final[bb] = finalInit;
            }
        }
        else
        {
            BitVector finalInit(this->_numElements, false);
            for (auto it = this->_gen.begin(); it != this->_gen.end(); ++it)
            {
                string bb = it->first;
                this->_final[bb] = finalInit;
            }
        }
        for (auto it = this->_gen.begin(); it != this->_gen.end(); ++it)
        {
            string bb = it->first;
            assert(this->_gen[bb].size() == this->_numElements);
            assert(this->_kill[bb].size() == this->_numElements);
        }
        assert(this->_numBasicBlocks == this->_basicBlockMap.size());
        this->_initialized = true;
    }

    template <typename T>
    void IterativeFramework<T>::Compute()
    {
        if (!this->_initialized)
            Init();
        map<string, BitVector> tempInitial = this->_initial;
        map<string, BitVector> tempFinal = this->_final;
        do
        {
            this->_updateInitialAndFinal(tempInitial, tempFinal);
            for (auto i = 0; i < this->_numBasicBlocks; ++i)
            {
                string bb = this->_basicBlockOrder[i];
                if (this->_meetOperator == Intersection)
                {
                    BitVector temp(this->_numElements, true);
                    if (i == 0)
                        temp &= this->_boundary;
                    for (auto it = this->_basicBlockMap[bb].begin(); it != this->_basicBlockMap[bb].end(); ++it)
                    {
                        string bb2 = *it;
                        temp &= tempFinal[bb2];
                    }
                    tempInitial[bb] = temp;
                }
                else
                {
                    BitVector temp(this->_numElements, false);
                    if (i == 0)
                        temp |= this->_boundary;
                    for (auto it = this->_basicBlockMap[bb].begin(); it != this->_basicBlockMap[bb].end(); ++it)
                    {
                        string bb2 = *it;
                        temp |= tempFinal[bb2];
                    }
                    tempInitial[bb] = temp;
                }
                tempFinal[bb] = this->TransferFunction(this->_gen[bb], this->_kill[bb], tempInitial[bb]);
            }
        } while (this->_hasChanged(tempInitial, tempFinal));
    }

    template <typename T>
    map<string, BitVector> IterativeFramework<T>::ReturnFinal()
    {
        return this->_final;
    }

    template <typename T>
    map<string, BitVector> IterativeFramework<T>::ReturnInitial()
    {
        return this->_initial;
    }
}