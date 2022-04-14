#include "pools.hpp"

// std::vector<Pools::Commutator> comm_;
// size_t Pools::Pool::poolNum = 0;

int main () {
    
    size_t num;
    std::cin >> num;

    // std::vector<Pools::Commutator> commVec;
    // commVec.reserve(num);

    // std::cout << "Reserve: " << commVec.size() << std::endl;
    // std::vector<Pools::Pool> pools(num, Pools::Pool(commVec));
    std::vector<Pools::Pool> pools;
    // pools.reserve(num);

    std::cout << "After creating of pools" << std::endl;

    std::cout << pools.size() << std::endl;

    for (size_t i = 0; i < num; ++i) {  

        std::cout << "---New iter:" << std::endl;
        // std::cout << " do emplace\n";
        pools.emplace_back();
        // std::cout << &pools[i] << " size = " << pools.size ();
        // std::cout << "\nthisCirc " << &(pools[i]) << std::endl;
    }
    std::cout << std::endl;
#if 0
    std::cout << "\t---Create a pool with 12 liters---" << std::endl;
    Pools::Pool pool(12);

    std::cout << "\t---Measure---" << std::endl;
    std::cout << pool.measure() << std::endl;

    std::cout << "\t---Add 10 liters---" << std::endl;
    pool.add(10);

    std::cout << "\t---Measure---" << std::endl;
    std::cout << pool.measure() << std::endl;

    std::cout << "\t---Create new pool with 120 liters---" << std::endl;
    Pools::Pool newPool (120);

    std::cout << "\t---Connect the first pool and the second pool with each other---" << std::endl;
    
    pool.connect (newPool);
    std::cout << "\t---Measure old one---" << std::endl;
    std::cout << pool.measure() << std::endl;
    std::cout << "\t---Measure new one---" << std::endl;
    std::cout << newPool.measure() << std::endl;
    std::cout << "\t---Create two pools with 50 and 100 liters and connect them---" << std::endl;

    Pools::Pool pool_3(50);
    Pools::Pool pool_4(100);

    pool_4.connect (pool_3);

    std::cout << "\t---Measure third one---" << std::endl;
    std::cout << pool_3.measure() << std::endl;
    std::cout << "\t---Measure forth one---" << std::endl;
    std::cout << pool_4.measure() << std::endl;

    std::cout << "\t---Connect pool_3 with the first pool---" << std::endl;

    pool_3.connect (pool);

    std::cout << "\t---Measure first one---" << std::endl;
    std::cout << pool.measure() << std::endl;
    std::cout << "\t---Measure second one---" << std::endl;
    std::cout << newPool.measure() << std::endl;
    std::cout << "\t---Measure third one---" << std::endl;
    std::cout << pool_3.measure() << std::endl;
    std::cout << "\t---Measure forth one---" << std::endl;
    std::cout << pool_4.measure() << std::endl;
    


#endif
    return 0;
}