#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;
using Clock = std::chrono::steady_clock;
struct Result {
    std::uint64_t lines = 0, matches = 0;
    bool operator==(const Result&) const = default;
};

Result search_file(const fs::path& file, const std::string& needle) {
    std::ifstream input(file, std::ios::binary);
    if (!input) throw std::runtime_error("Cannot open: " + file.string());
    Result result;
    std::string line;
    while (std::getline(input, line)) {
        ++result.lines;
        if (line.find(needle) != std::string::npos) ++result.matches;
    }
    if (input.bad()) throw std::runtime_error("Cannot read: " + file.string());
    return result;
}

std::vector<Result> scan(const std::vector<fs::path>& files,
                         const std::string& needle, unsigned workers) {
    if (workers == 0) throw std::invalid_argument("workers must be positive");
    std::vector<Result> results(files.size());
    if (workers == 1) {
        for (std::size_t i = 0; i < files.size(); ++i)
            results[i] = search_file(files[i], needle);
        return results;
    }
    std::vector<std::exception_ptr> errors(files.size());
    std::atomic<std::size_t> next_file{0};
    {
        std::vector<std::jthread> pool;
        workers = static_cast<unsigned>(std::min<std::size_t>(workers, files.size()));
        for (unsigned worker = 0; worker < workers; ++worker) {
            pool.emplace_back([&] {
                while (true) {
                    const auto i = next_file.fetch_add(1, std::memory_order_relaxed);
                    if (i >= files.size()) break;
                    try { results[i] = search_file(files[i], needle); }
                    catch (...) { errors[i] = std::current_exception(); }
                }
            });
        }
    }
    for (const auto& error : errors) if (error) std::rethrow_exception(error);
    return results;
}

double median(std::vector<double> values) {
    std::sort(values.begin(), values.end());
    const auto middle = values.size() / 2;
    return values.size() % 2 ? values[middle] : (values[middle - 1] + values[middle]) / 2;
}

int main(int argc, char** argv) {
    try {
        if (argc > 4) throw std::runtime_error("Usage: logscan [directory] [needle] [repeats:1..100]");
        const fs::path directory = argc > 1 ? argv[1] : "logs";
        const std::string needle = argc > 2 ? argv[2] : "ERROR";
        if (needle.empty()) throw std::runtime_error("Search string must not be empty");
        int repeats = 7;
        if (argc > 3) {
            std::size_t used = 0;
            repeats = std::stoi(argv[3], &used);
            if (used != std::string(argv[3]).size() || repeats < 1 || repeats > 100)
                throw std::runtime_error("Repeats must be between 1 and 100");
        }
        std::vector<fs::path> files;
        std::uintmax_t bytes = 0;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file() && (entry.path().extension() == ".log" || entry.path().extension() == ".txt")) {
                files.push_back(entry.path());
                bytes += entry.file_size();
            }
        }
        std::sort(files.begin(), files.end());
        if (files.empty()) throw std::runtime_error("No .log or .txt files in directory");
        // Independent sequential result and initial cache warm-up, outside timings.
        const auto reference = scan(files, needle, 1);
        std::uint64_t lines = 0, matches = 0;
        for (const auto& r : reference) { lines += r.lines; matches += r.matches; }
        constexpr std::array<unsigned, 4> workers{1, 2, 4, 8};
        std::array<std::vector<double>, 4> times;
        // Warm up each mode; timings include opening/reading and worker creation/join.
        for (auto w : workers)
            if (scan(files, needle, w) != reference) throw std::runtime_error("Warm-up mismatch");
        for (int round = 0; round < repeats; ++round) {
            for (std::size_t offset = 0; offset < workers.size(); ++offset) {
                const auto index = (static_cast<std::size_t>(round) + offset) % workers.size();
                const auto start = Clock::now();
                const auto result = scan(files, needle, workers[index]);
                const double ms = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
                if (result != reference) throw std::runtime_error("Per-file correctness check FAILED");
                times[index].push_back(ms);
            }
        }
        std::array<double, 4> medians;
        for (std::size_t i = 0; i < workers.size(); ++i) medians[i] = median(times[i]);
#if defined(_WIN32)
        std::cout << "OS: Windows\n";
#elif defined(__APPLE__)
        std::cout << "OS: macOS\n";
#elif defined(__linux__)
        std::cout << "OS: Linux\n";
#endif
#if defined(__clang__)
        std::cout << "Compiler: Clang " << __clang_version__ << '\n';
#elif defined(_MSC_VER)
        std::cout << "Compiler: MSVC " << _MSC_VER << '\n';
#elif defined(__GNUC__)
        std::cout << "Compiler: GCC " << __VERSION__ << '\n';
#endif
        std::cout << "hardware_concurrency (hint): " << std::thread::hardware_concurrency()
                  << "\nFiles: " << files.size() << "\nBytes: " << bytes
                  << "\nLines: " << lines << "\nMatching lines [" << needle << "]: " << matches
                  << "\nRepeats: " << repeats << " (warm cache, rotating order)\n\n"
                  << "Workers    Median ms    Speedup    Check\n";
        fs::create_directories("results");
        std::ofstream csv("results/benchmark.csv");
        if (!csv) throw std::runtime_error("Cannot create results/benchmark.csv");
        csv << "workers,median_ms,speedup,matching_lines,check\n";
        for (std::size_t i = 0; i < workers.size(); ++i) {
            if (medians[i] <= 0) throw std::runtime_error("Timer resolution insufficient for benchmark");
            const double speedup = medians[0] / medians[i];
            std::cout << std::setw(7) << workers[i] << std::fixed << std::setprecision(3)
                      << std::setw(13) << medians[i] << std::setw(11) << speedup << "x    PASS\n";
            csv << workers[i] << ',' << std::setprecision(9) << medians[i] << ','
                << speedup << ',' << matches << ",PASS\n";
        }
        csv.close();
        if (!csv) throw std::runtime_error("Cannot finish writing CSV");
        std::cout << "\nCHECK: all per-file results equal the sequential run.\nCSV: results/benchmark.csv\n";
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << '\n';
        return 1;
    }
}
