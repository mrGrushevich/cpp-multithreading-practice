#include <chrono>
#include <future>
#include <iostream>
#include <thread>

long long square_after_delay(int value, int delay_ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    return 1LL * value * value;
}

int main() {
    const auto start = std::chrono::steady_clock::now();
    auto first = std::async(std::launch::async, square_after_delay, 12, 500);
    auto second = std::async(std::launch::async, square_after_delay, 20, 500);
    const long long sum = first.get() + second.get();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    std::cout << "result=" << sum << "\nelapsed=" << elapsed
              << " ms\n(two 500 ms tasks)\n";
    return sum == 544 ? 0 : 1;
}
