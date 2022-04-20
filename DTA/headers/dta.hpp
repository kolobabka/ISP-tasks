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
#include <set>

#include <QBDI.h>
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

struct memInfo {

    bool enter_ = false;
    size_t size_;
    QBDI::rword rbp_;
    QBDI::rword mem_;
    const static inline auto cmp = [] (auto *x, auto *y) { return (x->address_ < y->address_); };
    static inline std::set <Vertex *, decltype(cmp)> vecs_; 
    bool coloredRegs_[19] = {}; 
    // Vertex* obj_;
};

struct Graph {

    Vertex* root_;
};

Graph* monitorDependences (DBI::DTA &dta);
QBDI::VMAction makeGraph (QBDI::VM *vm, QBDI::GPRState *gprState, 
                         QBDI::FPRState *fprState, void* data);
//----------------------------------------
//----------------------------------------
#endif