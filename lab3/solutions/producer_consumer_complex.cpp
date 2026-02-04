#include <atomic>
#include <thread>
#include <iostream>
#include <string>

// shared data
std::string shared_value;

// synchronization flags
std::atomic<bool> can_write{true};   // producer may write only if true
std::atomic<bool> ready{false};      // producer published data
std::atomic<bool> done{false};       // consumer has consumed

void producer(int id) {

    // wait for permission to write
    while (!atomic_load_explicit(&can_write, std::memory_order_acquire))
        std::this_thread::yield();

    // consumer must reset this
    atomic_store_explicit(&can_write, false, std::memory_order_release);

    // write shared data
    shared_value = "Data from Producer " + std::to_string(id);

    // publish written data
    atomic_store_explicit(&ready, true, std::memory_order_release);

    // std::cout << "[Producer " << id << "] wrote data\n";

    // wait for consumer to consume
    while (!atomic_load_explicit(&done, std::memory_order_acquire))
        std::this_thread::yield();

    // reset for next producer or stop
    atomic_store_explicit(&done, false, std::memory_order_release);
}

void consumer() {

    for (int i = 0; i < 2; i++) {   // expect two messages

        // wait for producer
        while (!atomic_load_explicit(&ready, std::memory_order_acquire))
            std::this_thread::yield();

        // read data
        std::cout << "    [Consumer] read: " << shared_value << "\n";

        // mark consumer done
        atomic_store_explicit(&ready, false, std::memory_order_release);
        atomic_store_explicit(&done, true,  std::memory_order_release);

        // allow next producer to write
        atomic_store_explicit(&can_write, true, std::memory_order_release);
    }
}

int main() {
    std::thread p0(producer, 0);
    std::thread p1(producer, 1);
    std::thread c(consumer);

    p0.join();
    p1.join();
    c.join();
}
