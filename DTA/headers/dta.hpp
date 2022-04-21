#ifndef __DTA_HPP__
//----------------------------------------
//----------------------------------------
#include <cassert>
#include <string>
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

namespace Memory {
    struct Memory {

        QBDI::rword address_;
        size_t size_;
    };  

    struct Data {

        QBDI::rword address_;
        QBDI::rword offset_;
    };

    struct Info {
        
        enum class FuncType {

            NEUTRAL = 0,
            CTOR = 1,
            DTOR = 2,
        };

        size_t numOfLog = 0;
        FuncType type_ = FuncType::NEUTRAL;

        static inline auto cmpVert = [] (auto x, auto y) { return (x.address_ < y.address_); };
        static inline auto cmpData = [] (auto x, auto y) { return (x.offset_ < y.offset_);   };

        static inline std::set <Memory, decltype(cmpVert)> pointers_; 
        static inline std::unordered_map<QBDI::rword, std::set<Data, decltype(cmpData)>> graph_;

        std::set <Memory, decltype(cmpVert)>::iterator find (const QBDI::rword address) const;

        void emplaceIntoGraph (const QBDI::rword lhsAddress, const QBDI::rword rhsAddress, const QBDI::rword offset);
        void eracePointer     (const QBDI::rword address);
        void graphUnwinding   (const QBDI::rword address);
        void dumpGraph        (const std::string &fileName) const;
    };
}
//----------------------------------------
//----------------------------------------
std::ostream&  operator << (std::ostream &out, const Memory::Data& vec);
Memory::Info   monitorDependences (DBI::DTA &dta);
QBDI::VMAction makeGraph (QBDI::VM *vm, QBDI::GPRState *gprState, 
                          QBDI::FPRState *fprState, void* data);
//----------------------------------------
//----------------------------------------
#endif