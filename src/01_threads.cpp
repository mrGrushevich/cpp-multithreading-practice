#include <iostream>
#include <syncstream>
#include <thread>
#include <vector>

void worker(int id) {
    std::osyncstream(std::cout) << "worker " << id << " started, id="
                               << std::this_thread::get_id() << '\n';
    std::osyncstream(std::cout) << "worker " << id << " finished\n";
}

int main() {
    std::cout << "hardware_concurrency=" << std::thread::hardware_concurrency() << '\n';
    {
        std::vector<std::jthread> threads;
        for (int i = 1; i <= 4; ++i) threads.emplace_back(worker, i);
        std::osyncstream(std::cout) << "main continues while workers run\n";
        // Destruction of the vector waits for every jthread.
    }
    std::cout << "all workers joined\n";
}
