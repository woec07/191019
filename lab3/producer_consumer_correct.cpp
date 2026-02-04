#include <thread>
#include <iostream>
#include <string>
#include <atomic>


std::string shared_value;

std::atomic<bool> write_done(false);
std::atomic<bool> write_allowed(true);
std::atomic<bool> read_by_consumer(false);

void producer(int id) {
    // while write_allowed flag = false -> busy waiting
    while(!atomic_load_explicit(&write_allowed, std::memory_order_acquire));
    
    // as soon as write is allowed, set flag to false so no other thread can access it

    // WHY MEMORY ORDER RELEASE?
    atomic_store_explicit(&write_allowed, false, std::memory_order_release);

    // First write
    shared_value = "Data from Producer " + std::to_string(id);
    std::cout << "[Producer " << id << "] wrote data\n";

    // set write_done flag, so consumer now can now that write was done
    atomic_store_explicit(&write_done, true, std::memory_order_release);
    //producer has to wait until consumer has read
    while(!atomic_load_explicit(&read_by_consumer, std::memory_order_acquire));

    // now consumer has read, so we reset read_by_consumer and write_done to false and write_allowed to true
    atomic_store_explicit(&read_by_consumer, false, std::memory_order_release);
    atomic_store_explicit(&write_allowed, true, std::memory_order_release);
}

void consumer() {
    for (int i=0; i<2; i++){
        while(!atomic_load_explicit(&write_done, std::memory_order_acquire));
        std::cout << "    [Consumer] read: " << shared_value << "\n";
        atomic_store_explicit(&write_done, false, std::memory_order_release);
        atomic_store_explicit(&read_by_consumer, true, std::memory_order_release);

    }
}

int main() {
    std::thread p0(producer,0);
    std::thread p1(producer,1);
    std::thread c(consumer);

    p0.join();
    p1.join();
    c.join();
}

