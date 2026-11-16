#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

int main() {
    constexpr int workers = 8, iterations = 500'000;
    int protected_counter = 0;
    std::atomic<int> atomic_counter{0};
    std::mutex mutex;
    auto measure = [](auto task) {
        const auto start = std::chrono::steady_clock::now();
        {
            std::vector<std::jthread> threads;
            for (int i = 0; i < workers; ++i) threads.emplace_back(task);
        }
        return std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - start).count();
    };
    const double mutex_ms = measure([&] {
        for (int n = 0; n < iterations; ++n) {
            std::lock_guard<std::mutex> lock(mutex);
            ++protected_counter;
        }
    });
    const double atomic_ms = measure([&] {
        for (int n = 0; n < iterations; ++n)
            atomic_counter.fetch_add(1, std::memory_order_relaxed);
    });
    std::cout << std::fixed << std::setprecision(3)
              << "mutex result=" << protected_counter << " time_ms=" << mutex_ms << '\n'
              << "atomic result=" << atomic_counter.load() << " time_ms=" << atomic_ms << '\n';
    const bool ok = protected_counter == workers * iterations
                 && atomic_counter.load() == workers * iterations;
    std::cout << (ok ? "PASS\n" : "FAIL\n");
    return ok ? 0 : 1;
}
