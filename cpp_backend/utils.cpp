#include "utils.hpp"
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <sstream>

namespace utils {

bool is_safe_directory(const std::filesystem::path& path) {
    // Prevent operations on root directories
    if (path == "/" || path == "C:\\" || path == "D:\\") {
        return false;
    }
    
    // Prevent operations on system directories
    if (is_system_directory(path)) {
        return false;
    }
    
    return true;
}

bool is_system_directory(const std::filesystem::path& path) {
    std::vector<std::string> system_dirs = {
        "/bin", "/sbin", "/usr", "/etc", "/var", "/lib", "/opt",
        "C:\\Windows", "C:\\Program Files", "C:\\Program Files (x86)"
    };
    
    std::string path_str = path.string();
    for (const auto& sys_dir : system_dirs) {
        if (path_str.find(sys_dir) == 0) {
            return true;
        }
    }
    
    return false;
}

bool requires_confirmation(const std::string& action, const std::filesystem::path& path) {
    if (action == "delete") {
        return true;
    }
    
    // Check if path contains many files
    try {
        size_t file_count = 0;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
            if (++file_count > 100) {
                return true;
            }
        }
    } catch (...) {
        // If we can't scan, assume it's safe
    }
    
    return false;
}

std::vector<std::filesystem::path> scan_directory(const std::filesystem::path& dir_path) {
    std::vector<std::filesystem::path> files;
    
    if (!std::filesystem::exists(dir_path) || !std::filesystem::is_directory(dir_path)) {
        return files;
    }
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // Silently skip directories we can't access
    }
    
    return files;
}

std::vector<std::filesystem::path> scan_directory_recursive(const std::filesystem::path& dir_path) {
    std::vector<std::filesystem::path> files;
    
    if (!std::filesystem::exists(dir_path) || !std::filesystem::is_directory(dir_path)) {
        return files;
    }
    
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(dir_path)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // Silently skip directories we can't access
    }
    
    return files;
}

bool matches_pattern(const std::string& filename, const std::string& pattern) {
    // Simple pattern matching for backward compatibility
    if (pattern.empty()) return true;
    
    // Check if it's a glob pattern
    if (pattern.find('*') != std::string::npos || pattern.find('?') != std::string::npos) {
        return matches_glob_pattern(filename, pattern);
    }
    
    // Simple extension matching
    if (pattern[0] == '.') {
        return filename.ends_with(pattern);
    }
    
    // Exact filename matching
    return filename == pattern;
}

bool matches_glob_pattern(const std::string& filename, const std::string& pattern) {
    // Convert glob pattern to regex
    std::string regex_pattern = pattern;
    
    // Handle ** recursive patterns
    if (regex_pattern.find("**") != std::string::npos) {
        // Replace ** with .* for regex
        size_t pos = 0;
        while ((pos = regex_pattern.find("**", pos)) != std::string::npos) {
            regex_pattern.replace(pos, 2, ".*");
            pos += 2;
        }
    }
    
    // Escape regex special characters except * and ?
    std::string escaped;
    for (char c : regex_pattern) {
        if (c == '*' || c == '?') {
            if (c == '*') escaped += ".*";
            else if (c == '?') escaped += ".";
        } else if (std::string("()[]{}.+^$|\\").find(c) != std::string::npos) {
            escaped += "\\" + std::string(1, c);
        } else {
            escaped += c;
        }
    }
    
    try {
        std::regex re(escaped, std::regex::icase);
        return std::regex_match(filename, re);
    } catch (const std::regex_error&) {
        // If regex compilation fails, fall back to simple matching
        return filename.find(pattern) != std::string::npos;
    }
}

std::filesystem::path expand_path(const std::string& path_string) {
    if (path_string.starts_with("~/")) {
        const char* home = std::getenv("HOME");
        if (home) {
            return std::filesystem::path(home) / path_string.substr(2);
        }
    }
    return std::filesystem::path(path_string);
}

std::string get_human_readable_size(uintmax_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit_index = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unit_index < 4) {
        size /= 1024.0;
        unit_index++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << size << " " << units[unit_index];
    return oss.str();
}

bool validate_source_path(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        return false;
    }
    
    if (!std::filesystem::is_directory(path)) {
        return false;
    }
    
    return is_safe_directory(path);
}

bool validate_destination_path(const std::filesystem::path& path) {
    // For destination, we allow non-existent paths (they will be created)
    if (std::filesystem::exists(path)) {
        if (!std::filesystem::is_directory(path)) {
            return false;
        }
    }
    
    return is_safe_directory(path);
}

} // namespace utils 