#include <atomic>
#include <thread>
#include <iostream>
#include <string>

// shared data
std::string shared_value;

// synchronization flags
std::atomic<bool> can_write{true};   // producer may write only if true
std::atomic<bool> ready{false};      // producer published data

void producer(int id) {

    bool expected = true;
    while (!can_write.compare_exchange_strong(expected, false, 
                                             std::memory_order_acquire, 
                                             std::memory_order_relaxed)) {
        expected = true; 
        std::this_thread::yield();
    }

    
    // write shared data
    shared_value = "Data from Producer " + std::to_string(id);

    // publish written data
    atomic_store_explicit(&ready, true, std::memory_order_release);

    
   
}

void consumer() {

    for (int i = 0; i < 6; i++) {

        // wait for producer
        while (!atomic_load_explicit(&ready, std::memory_order_acquire))
            std::this_thread::yield();

        // read data
        std::cout << "    [Consumer] read: " << shared_value << "\n";

        // mark consumer done
        atomic_store_explicit(&ready, false, std::memory_order_release);

        atomic_store_explicit(&can_write, true, std::memory_order_release);
    }
    
    
}

int main() {
    std::thread p0(producer, 0);
    std::thread p1(producer, 1);
    std::thread p2(producer, 2);
     std::thread c(consumer);
    std::thread p3(producer, 3);
    std::thread p4(producer, 4);
    std::thread p5(producer, 5);
    
   

    p0.join();
    p1.join();
    p2.join();
    c.join(); 
    p3.join();
    p4.join();
    p5.join();
    
    
    std::cout << "All threads finished successfully.\n";
}