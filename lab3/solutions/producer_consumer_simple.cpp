#include <atomic>
#include <thread>
#include <iostream>
#include <string>


std::atomic<std::string*> ptr{nullptr};
std::atomic<bool> ready(false);

void producer() {

    std::string* msg = new std::string("Hello from Producer!");

    atomic_store_explicit(&ptr, msg, std::memory_order_relaxed);

    atomic_store_explicit(&ready, true, std::memory_order_release);
}

void consumer() {
    std::string* p = nullptr;


    while (!atomic_load_explicit(&ready, std::memory_order_acquire)) {
        // busy-wait
    }

    p = atomic_load_explicit(&ptr, std::memory_order_acquire);

    std::cout << "Consumer received message: " << *p << std::endl;

    delete p;
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);
    t1.join();
    t2.join();
    return 0;
}
