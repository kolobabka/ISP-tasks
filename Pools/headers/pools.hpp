#ifndef __POOLS_HPP__
#define __POOLS_HPP__

#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <unordered_set>

namespace Pools {

    class Pool;
    struct IPool {

        virtual long measure () const = 0;
        virtual void connect (IPool &pool) = 0;
        virtual void add (long water) = 0;
        virtual ~IPool () {}
    };

    struct Commutator final {

        std::vector<Pool *> pools_;
        long totalVol_ = 0;

        Commutator (Pool* pool) {
            std::cout << "dobavil???? " << this << std::endl;
            pools_.push_back(pool);
            std::cout << "da da dobavil\n";
        
        }

        ~Commutator () {
            std::cout << "CUMmutatorD = " << this << std::endl;
        }

        inline long getVolumeOfPool () const noexcept {return totalVol_ / pools_.size();}
        inline void addWater (long water)    noexcept {totalVol_ += water;     } 
    };
}

namespace std {


    template <> struct hash<Pools::Commutator> {

        size_t operator() (const Pools::Commutator &comm) const {
            
            return hash<unsigned long long>()(reinterpret_cast<unsigned long long>(&(comm.pools_)));
        }
    };
}

namespace Pools {

    struct Container final {

        static inline std::unordered_set<Commutator> comm_;
        // inline std::unordered_set<Commutator> & get () const { return comm_; }
    };  

    class Pool final : public IPool {

    public:

        Container con_;
        std::unordered_set<Commutator>::const_iterator itKey;
    public:
        Pool () { 
            
            std::cout << "Pool ctor " << this << std::endl;
            // std::cout << "Size " << con_.comm_.size() << std::endl;
            // std::cout << "thisC = " << this << std::endl;

            // Commutator toInsert {this};
            // con_.comm_.insert (toInsert);

            // itKey = con_.comm_.find(toInsert);
            // std::cout << "Alive" << std::endl;
        }

        Pool (const Pool& rhs) = delete;
        Pool& operator = (const Pool& rhs) = default; 
        Pool (Pool&& rhs) {
            
            std::cout << "Move ctr: " << &rhs << std::endl;
            std::cout << "This " << this << std::endl;
        }

        ~Pool () {
            std::cout << "Pool dtor " << this << std::endl;

            // std::cout << "thisD = " << this << std::endl;
        }

        inline long measure () const noexcept override { return itKey->getVolumeOfPool(); } 
        void connect (IPool &pool) override {
            
            Pool& toConnect = static_cast<Pool&>(pool);
            
        }
        void add (long water) override { 
            
            auto copy = *itKey;
            Container::comm_.erase(itKey); 
            copy.addWater (water);
            Container::comm_.insert(copy);
            itKey = Container::comm_.find (copy);
        }
    };

    bool operator == (const Commutator &lhs, const Commutator &rhs);

}




#endif