#include <chrono>
#include <iostream>
#include <stop_token>
#include <thread>

void polling_worker(std::stop_token stop) {
    int iteration = 0;
    while (!stop.stop_requested()) {
        std::cout << "iteration " << ++iteration << '\n';
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "worker stopped cleanly\n";
}

int main() {
    std::jthread worker(polling_worker);
    std::this_thread::sleep_for(std::chrono::milliseconds(350));
    worker.request_stop();
    // The destructor joins. The number of printed iterations is not guaranteed.
}
