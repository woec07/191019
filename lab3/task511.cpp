#include <omp.h>
#include <atomic>
#include <thread>
#include <iostream>
#include <string>

std::atomic<std::string*> ptr{nullptr};
std::atomic<bool> ready(false);

void produce(){
    std::string* msg = new std::string("Message");
    atomic_store_explicit(&ptr, msg, std::memory_order_relaxed);
    atomic_store_explicit(&ready, true, std::memory_order_release);
}

void consume(){
    std::string* p = nullptr;

    while(atomic_load_explicit(&ready,std::memory_order_acquire) == false);
    p = atomic_load_explicit(&ptr, std::memory_order_acquire);
    std::cout << "Value: " << *p << std::endl;
    delete p;
}

int main(){
    std::thread t1(produce);
    std::thread t2(consume);

    t1.join();
    t2.join();

}