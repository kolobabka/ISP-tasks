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
#include <iterator>
#include <utility>
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

    struct Info {
        
        enum class FuncType {

            NEUTRAL = 0,
            SOURCE = 1,
            SINK = 2,
        };

        template <typename lhsIt, typename rhsIt, typename val>
        struct changeShadowFunctor {

            changeShadowFunctor () {}
        };

        FuncType type_ = FuncType::NEUTRAL;
        QBDI::rword size_  = 0;

        static inline auto cmpMem = [] (auto x, auto y) { return (x.address_ < y.address_); };
        static inline std::set <Memory, decltype (cmpMem)> shadowMem_;
        static inline std::set <int16_t> shadowReg_; 

        using memIt = std::set <Memory>::iterator;
        using regIt = std::set <int16_t>::iterator;

        memIt find (const QBDI::rword address) const;
    };

    using memIt = std::set <Memory>::iterator;
    using regIt = std::set <int16_t>::iterator;    

    template <> 
    struct Info::changeShadowFunctor<Info::memIt, Info::regIt, QBDI::rword> {

        changeShadowFunctor (Info::memIt lhs, Info::regIt rhs, QBDI::rword address) {
                                        
            auto lhsEnd  = shadowMem_.end ();  
            auto rhsEnd  = shadowReg_.end ();
    
            if (lhs == lhsEnd && rhs != rhsEnd) 
                shadowMem_.emplace (address);
            else
                if (rhs == rhsEnd && lhs != lhsEnd) 
                    shadowMem_.erase (lhs);
        }
    };

    template <> 
    struct Info::changeShadowFunctor<Info::regIt, Info::memIt, int16_t> {

        changeShadowFunctor (Info::regIt lhs, Info::memIt rhs, int16_t lhsIdx) {
            
            auto lhsEnd  = shadowReg_.end ();  
            auto rhsEnd  = shadowMem_.end ();
    
            if (lhs == lhsEnd && rhs != rhsEnd) 
                shadowReg_.emplace (lhsIdx);
            else 
                if (rhs == rhsEnd && lhs != lhsEnd) 
                    shadowReg_.erase (lhs);
        }
    };

    template <>
    struct Info::changeShadowFunctor<Info::regIt, Info::regIt, int16_t> {

        changeShadowFunctor (Info::regIt lhs, Info::regIt rhs, int16_t lhsIdx) {
            
            auto end = shadowReg_.end ();

            if (lhs == end && rhs != end)         
            	shadowReg_.emplace (lhsIdx);
            else 
                if (rhs == end && lhs != end) 
                    shadowReg_.erase (lhs);
        }
    };
}


//----------------------------------------
//----------------------------------------
std::ostream&  operator << (std::ostream &out, const Memory::Memory& vec);
Memory::Info   leakDetector (DBI::DTA &dta);
//----------------------------------------
//----------------------------------------
#endif