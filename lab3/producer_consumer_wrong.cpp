#include <thread>
#include <iostream>
#include <string>

std::string shared_value;

void producer(int id) {
    // First write
    shared_value = "Data from Producer " + std::to_string(id);yy
    std::cout << "[Producer " << id << "] wrote data\n";
}

void consumer() {
    for (int i=0; i<2; i++){
        std::cout << "    [Consumer] read: " << shared_value << "\n";
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

