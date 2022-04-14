#include "pools.hpp"

namespace Pools {
    bool operator == (const Commutator &lhs, const Commutator &rhs) {

        if (lhs.pools_ == rhs.pools_)
            return true;

        return false;
    }
}