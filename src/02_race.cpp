#include <iostream>
#include <thread>
#include <vector>

// INTENTIONALLY INCORRECT: simultaneous unsynchronised writes cause a data race.
// Any result is possible, including the expected one. This is undefined behaviour.
int main() {
    int counter = 0;
    {
        std::vector<std::jthread> threads;
        for (int i = 0; i < 8; ++i) {
            threads.emplace_back([&] {
                for (int n = 0; n < 1'000'000; ++n) ++counter;
            });
        }
    }
    std::cout << "WARNING: intentional data race; undefined behaviour\n"
              << "expected=" << 8'000'000 << '\n'
              << "actual=" << counter << '\n';
}
