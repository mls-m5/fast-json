#pragma once

#include <filesystem>
#include <fstream>
#include <istream>
#include <stdexcept>
#include <string>

namespace json {

inline std::string read_file_content(const std::filesystem::path &filename) {
    std::ifstream input_file(filename);
    if (input_file.is_open()) {
        std::stringstream buffer;
        buffer << input_file.rdbuf();
        return buffer.str();
    }
    else {
        throw std::runtime_error{"Failed to open " + filename.string()};
        return "";
    }
}

inline std::string read_stream_content(std::istream &stream) {
    std::stringstream buffer{};
    buffer << stream.rdbuf();
    return buffer.str();
}

} // namespace json
