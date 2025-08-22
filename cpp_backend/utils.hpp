#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <regex>
#include <chrono>
#include <optional>

namespace utils {

// File operation result structure
struct FileOpResult {
    bool success = false;
    std::string operation = "";
    std::string message = "";
    std::string error_message = "";
    size_t files_scanned = 0;
    size_t files_matched = 0;
    size_t files_affected = 0;
    std::vector<std::string> errors;
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;
};

// Safety check functions
bool is_safe_directory(const std::filesystem::path& path);
bool is_system_directory(const std::filesystem::path& path);
bool requires_confirmation(const std::string& action, const std::filesystem::path& path);

// File scanning and pattern matching
std::vector<std::filesystem::path> scan_directory(const std::filesystem::path& dir_path);
std::vector<std::filesystem::path> scan_directory_recursive(const std::filesystem::path& dir_path);
bool matches_pattern(const std::string& filename, const std::string& pattern);
bool matches_glob_pattern(const std::string& filename, const std::string& pattern);

// Path utilities
std::filesystem::path expand_path(const std::string& path_string);
std::string get_human_readable_size(uintmax_t bytes);

// Validation functions
bool validate_source_path(const std::filesystem::path& path);
bool validate_destination_path(const std::filesystem::path& path);

} // namespace utils
