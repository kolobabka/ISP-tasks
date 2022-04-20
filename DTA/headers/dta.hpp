#ifndef __DTA_HPP__
//----------------------------------------
//----------------------------------------
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <exception>
#include <unordered_map>
#include <set>
#include <QBDI.h>
#include <fstream>
//----------------------------------------
//----------------------------------------
namespace DBI {

    class DTA {

        uint8_t *fakestack_;
        static inline size_t STACK_SIZE = 0x100000; 

    public:
        QBDI::VM vm_;
        QBDI::GPRState *state_;
        DTA () {

            state_ = vm_.getGPRState();
            QBDI::allocateVirtualStack (state_, STACK_SIZE, &fakestack_);
        }

        ~DTA () { QBDI::alignedFree(fakestack_); }
    };
}

struct Vertex {

    QBDI::rword address_;
    size_t size_;
};  

namespace std {


    template <> struct hash<Vertex> {

        size_t operator() (const Vertex &vert) const {
            
            return hash<unsigned long long>()(static_cast<unsigned long long>(vert.address_));
        }
    };
}

struct memInfo {

    QBDI::rword rbp_ = 0;
    const static inline auto cmpVert = [] (auto x, auto y) { return (x.address_ < y.address_); };
    const static inline auto cmpAddress = [] (auto x, auto y) { return (y.address_ == x.address_); };
    static inline std::set <Vertex, decltype(cmpVert)> vecs_; 

    static inline std::unordered_map<Vertex, std::set<Vertex, decltype(cmpVert)>, std::hash<Vertex>, decltype(cmpAddress)> graph_;

    std::set <Vertex, decltype(cmpVert)>::iterator find (const QBDI::rword address) const;

    void dumpGraph (const char *fileName) const;
};

std::ostream& operator << (std::ostream &out, const Vertex& vec);
void monitorDependences (DBI::DTA &dta);
QBDI::VMAction makeGraph (QBDI::VM *vm, QBDI::GPRState *gprState, 
                         QBDI::FPRState *fprState, void* data);
//----------------------------------------
//----------------------------------------
#endif