#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;
int main(int argc, char** argv) {
    try {
        if (argc > 2) throw std::runtime_error("Usage: generate_logs [output_directory]");
        const fs::path directory = argc == 2 ? argv[1] : "logs";
        if (fs::exists(directory) && (!fs::is_directory(directory) || !fs::is_empty(directory)))
            throw std::runtime_error("Output directory must be empty; existing data will not be overwritten.");
        fs::create_directories(directory);
        constexpr int files = 32, lines = 50'000, bytes_per_line = 196;
        for (int f = 0; f < files; ++f) {
            std::ostringstream name;
            name << "log_" << std::setfill('0') << std::setw(2) << f << ".log";
            std::ofstream out(directory / name.str(), std::ios::binary);
            if (!out) throw std::runtime_error("Cannot create log file");
            for (int n = 1; n <= lines; ++n) {
                std::string line = "file=" + std::to_string(f) + " line=" + std::to_string(n)
                    + (n % 20 == 0 ? " ERROR simulated failure " : " INFO normal operation ");
                line.resize(bytes_per_line - 1, 'x');
                out << line << '\n';
            }
            out.close();
            if (!out) throw std::runtime_error("Error writing log file");
        }
        std::cout << "Generated files: 32\nLines: 1600000\nBytes: 313600000\n"
                     "Expected matching lines [ERROR]: 80000\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << '\n';
        return 1;
    }
}
