#include "actions.hpp"
#include <iostream>
#include <algorithm>
#include <chrono>

namespace actions {

utils::FileOpResult move_files(const Command& cmd) {
    utils::FileOpResult result;
    result.operation = "move";
    result.start_time = std::chrono::high_resolution_clock::now();
    
    try {
        std::filesystem::path source_path = utils::expand_path(cmd.source);
        std::filesystem::path dest_path = utils::expand_path(cmd.destination);
        
        // Safety checks
        if (!utils::is_safe_directory(source_path)) {
            result.success = false;
            result.error_message = "Source directory is not safe to operate on";
            return result;
        }
        
        if (!utils::is_safe_directory(dest_path)) {
            result.success = false;
            result.error_message = "Destination directory is not safe to operate on";
            return result;
        }
        
        // Scan for matching files
        std::vector<std::filesystem::path> files;
        if (cmd.recursive) {
            files = utils::scan_directory_recursive(source_path);
        } else {
            files = utils::scan_directory(source_path);
        }
        
        // Filter files by pattern
        std::vector<std::filesystem::path> matching_files;
        for (const auto& file : files) {
            if (utils::matches_pattern(file.filename().string(), cmd.pattern)) {
                matching_files.push_back(file);
            }
        }
        
        result.files_scanned = files.size();
        result.files_matched = matching_files.size();
        
        if (cmd.dry_run) {
            result.message = "Would move " + std::to_string(matching_files.size()) + " files";
            result.success = true;
            return result;
        }
        
        // Move files
        int moved_count = 0;
        for (const auto& file : matching_files) {
            try {
                std::filesystem::path dest_file = dest_path / file.filename();
                
                if (cmd.verbose) {
                    std::cerr << "Moving: " << file << " → " << dest_file << std::endl;
                }
                
                std::filesystem::rename(file, dest_file);
                moved_count++;
                
            } catch (const std::exception& e) {
                if (cmd.verbose) {
                    std::cerr << "Error moving " << file << ": " << e.what() << std::endl;
                }
                result.errors.push_back("Failed to move " + file.string() + ": " + e.what());
            }
        }
        
        result.files_affected = moved_count;
        result.message = "Successfully moved " + std::to_string(moved_count) + " files";
        result.success = true;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = "Move operation failed: " + std::string(e.what());
    }
    
    result.end_time = std::chrono::high_resolution_clock::now();
    return result;
}

utils::FileOpResult copy_files(const Command& cmd) {
    utils::FileOpResult result;
    result.operation = "copy";
    result.start_time = std::chrono::high_resolution_clock::now();
    
    try {
        std::filesystem::path source_path = utils::expand_path(cmd.source);
        std::filesystem::path dest_path = utils::expand_path(cmd.destination);
        
        // Safety checks
        if (!utils::is_safe_directory(source_path)) {
            result.success = false;
            result.error_message = "Source directory is not safe to operate on";
            return result;
        }
        
        if (!utils::is_safe_directory(dest_path)) {
            result.success = false;
            result.error_message = "Destination directory is not safe to operate on";
            return result;
        }
        
        // Scan for matching files
        std::vector<std::filesystem::path> files;
        if (cmd.recursive) {
            files = utils::scan_directory_recursive(source_path);
        } else {
            files = utils::scan_directory(source_path);
        }
        
        // Filter files by pattern
        std::vector<std::filesystem::path> matching_files;
        for (const auto& file : files) {
            if (utils::matches_pattern(file.filename().string(), cmd.pattern)) {
                matching_files.push_back(file);
            }
        }
        
        result.files_scanned = files.size();
        result.files_matched = matching_files.size();
        
        if (cmd.dry_run) {
            result.message = "Would copy " + std::to_string(matching_files.size()) + " files";
            result.success = true;
            return result;
        }
        
        // Copy files
        int copied_count = 0;
        for (const auto& file : matching_files) {
            try {
                std::filesystem::path dest_file = dest_path / file.filename();
                
                if (cmd.verbose) {
                    std::cerr << "Copying: " << file << " → " << dest_file << std::endl;
                }
                
                std::filesystem::copy_file(file, dest_file, std::filesystem::copy_options::overwrite_existing);
                copied_count++;
                
            } catch (const std::exception& e) {
                if (cmd.verbose) {
                    std::cerr << "Error copying " << file << ": " << e.what() << std::endl;
                }
                result.errors.push_back("Failed to copy " + file.string() + ": " + e.what());
            }
        }
        
        result.files_affected = copied_count;
        result.message = "Successfully copied " + std::to_string(copied_count) + " files";
        result.success = true;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = "Copy operation failed: " + std::string(e.what());
    }
    
    result.end_time = std::chrono::high_resolution_clock::now();
    return result;
}

utils::FileOpResult delete_files(const Command& cmd) {
    utils::FileOpResult result;
    result.operation = "delete";
    result.start_time = std::chrono::high_resolution_clock::now();
    
    try {
        std::filesystem::path source_path = utils::expand_path(cmd.source);
        
        // Safety checks
        if (!utils::is_safe_directory(source_path)) {
            result.success = false;
            result.error_message = "Source directory is not safe to operate on";
            return result;
        }
        
        // Scan for matching files
        std::vector<std::filesystem::path> files;
        if (cmd.recursive) {
            files = utils::scan_directory_recursive(source_path);
        } else {
            files = utils::scan_directory(source_path);
        }
        
        // Filter files by pattern
        std::vector<std::filesystem::path> matching_files;
        for (const auto& file : files) {
            if (utils::matches_pattern(file.filename().string(), cmd.pattern)) {
                matching_files.push_back(file);
            }
        }
        
        result.files_scanned = files.size();
        result.files_matched = matching_files.size();
        
        if (cmd.dry_run) {
            result.message = "Would delete " + std::to_string(matching_files.size()) + " files";
            result.success = true;
            return result;
        }
        
        // Delete files
        int deleted_count = 0;
        for (const auto& file : matching_files) {
            try {
                if (cmd.verbose) {
                    std::cerr << "Deleting: " << file << std::endl;
                }
                
                std::filesystem::remove(file);
                deleted_count++;
                
            } catch (const std::exception& e) {
                if (cmd.verbose) {
                    std::cerr << "Error deleting " << file << ": " << e.what() << std::endl;
                }
                result.errors.push_back("Failed to delete " + file.string() + ": " + e.what());
            }
        }
        
        result.files_affected = deleted_count;
        result.message = "Successfully deleted " + std::to_string(deleted_count) + " files";
        result.success = true;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = "Delete operation failed: " + std::string(e.what());
    }
    
    result.end_time = std::chrono::high_resolution_clock::now();
    return result;
}

utils::FileOpResult create_folder(const Command& cmd) {
    utils::FileOpResult result;
    result.operation = "create_folder";
    result.start_time = std::chrono::high_resolution_clock::now();
    
    try {
        std::filesystem::path folder_path = utils::expand_path(cmd.destination);
        
        // Safety checks
        if (!utils::is_safe_directory(folder_path.parent_path())) {
            result.success = false;
            result.error_message = "Parent directory is not safe to operate on";
            return result;
        }
        
        if (cmd.dry_run) {
            result.message = "Would create folder: " + folder_path.string();
            result.success = true;
            return result;
        }
        
        if (cmd.verbose) {
            std::cerr << "Creating folder: " << folder_path << std::endl;
        }
        
        std::filesystem::create_directories(folder_path);
        
        result.files_affected = 1;
        result.message = "Successfully created folder: " + folder_path.string();
        result.success = true;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.error_message = "Create folder operation failed: " + std::string(e.what());
    }
    
    result.end_time = std::chrono::high_resolution_clock::now();
    return result;
}

utils::FileOpResult execute_command(const Command& cmd) {
    if (cmd.verbose) {
        std::cerr << "Executing command: " << command_to_string(cmd) << std::endl;
    }
    
    if (!validate_command(cmd)) {
        utils::FileOpResult result;
        result.success = false;
        result.error_message = "Invalid command";
        return result;
    }
    
    utils::FileOpResult result;
    
    if (cmd.action == "move") {
        result = move_files(cmd);
    } else if (cmd.action == "copy") {
        result = copy_files(cmd);
    } else if (cmd.action == "delete") {
        result = delete_files(cmd);
    } else if (cmd.action == "create_folder") {
        result = create_folder(cmd);
    } else {
        result.success = false;
        result.error_message = "Unknown action: " + cmd.action;
    }
    
    if (cmd.verbose) {
        std::cerr << "Operation completed: " << (result.success ? "SUCCESS" : "FAILED") << std::endl;
        if (!result.success && !result.error_message.empty()) {
            std::cerr << "Error: " << result.error_message << std::endl;
        }
    }
    
    return result;
}

std::string command_to_string(const Command& cmd) {
    std::string result = cmd.action;
    
    if (!cmd.pattern.empty()) {
        result += " files matching '" + cmd.pattern + "'";
    }
    
    if (!cmd.source.empty()) {
        result += " from '" + cmd.source + "'";
    }
    
    if (!cmd.destination.empty()) {
        result += " to '" + cmd.destination + "'";
    }
    
    if (cmd.recursive) {
        result += " (recursive)";
    }
    
    if (cmd.dry_run) {
        result += " (dry-run)";
    }
    
    return result;
}

bool validate_command(const Command& cmd) {
    if (cmd.action.empty()) {
        return false;
    }
    
    if (cmd.action == "move" || cmd.action == "copy") {
        return !cmd.source.empty() && !cmd.destination.empty();
    }
    
    if (cmd.action == "delete") {
        return !cmd.source.empty();
    }
    
    if (cmd.action == "create_folder") {
        return !cmd.destination.empty();
    }
    
    return false;
}

} // namespace actions
