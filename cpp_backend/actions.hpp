#pragma once

#include <string>
#include <filesystem>
#include "utils.hpp"

namespace actions {

// Command structure received from Python frontend
struct Command {
    std::string action;           // "move", "copy", "delete", "create_folder"
    std::string pattern;          // file pattern (".jpg", "*.png", "**/*.txt", etc.)
    std::string source;           // source directory
    std::string destination;      // destination (for move/copy/create_folder)
    bool dry_run = false;         // preview mode
    bool force = false;           // skip confirmations
    bool recursive = false;       // scan subdirectories recursively
    bool verbose = false;         // detailed output
};

// File operation functions
utils::FileOpResult move_files(const Command& cmd);
utils::FileOpResult copy_files(const Command& cmd);
utils::FileOpResult delete_files(const Command& cmd);
utils::FileOpResult create_folder(const Command& cmd);

// Main execution function
utils::FileOpResult execute_command(const Command& cmd);

// Utility functions
std::string command_to_string(const Command& cmd);
bool validate_command(const Command& cmd);

} // namespace actions
