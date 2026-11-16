#include <filesystem>
#include <iostream>
#include <thread>

int main() {
#if defined(_WIN32)
    const char* platform = "Windows";
#elif defined(__APPLE__)
    const char* platform = "macOS";
#elif defined(__linux__)
    const char* platform = "Linux";
#else
    const char* platform = "Other";
#endif
    std::filesystem::path example = std::filesystem::path("logs") / "result.txt";
    example.make_preferred();
    std::cout << "platform=" << platform << '\n'
              << "path separator=" << static_cast<char>(std::filesystem::path::preferred_separator) << '\n'
              << "example=" << example.string() << '\n'
              << "logical processors (hint)=" << std::thread::hardware_concurrency() << '\n';
}
