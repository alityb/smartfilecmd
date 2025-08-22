#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include "actions.hpp"

using json = nlohmann::json;

int main() {
    std::string input;
    
    // Read JSON command from stdin
    std::getline(std::cin, input);
    
    try {
        // Parse JSON command
        json j = json::parse(input);
        
        // Debug: print the parsed JSON to stderr
        std::cerr << "DEBUG: Parsed JSON: " << j.dump(2) << std::endl;
        
        // Convert JSON to Command struct
        actions::Command cmd;
        
        // Debug: check each field to stderr
        std::cerr << "DEBUG: action field: " << (j.contains("action") ? "exists" : "missing") << std::endl;
        if (j.contains("action")) {
            std::cerr << "DEBUG: action value: " << j["action"] << std::endl;
        }
        
        // Safe assignment with type checking
        if (j.contains("action") && j["action"].is_string()) {
            cmd.action = j["action"].get<std::string>();
        } else {
            std::cerr << "ERROR: action field is missing or not a string" << std::endl;
            return 1;
        }
        
        if (j.contains("pattern") && j["pattern"].is_string()) {
            cmd.pattern = j["pattern"].get<std::string>();
        } else {
            cmd.pattern = "";
        }
        
        if (j.contains("source") && j["source"].is_string()) {
            cmd.source = j["source"].get<std::string>();
        } else {
            cmd.source = "";
        }
        
        if (j.contains("destination") && j["destination"].is_string()) {
            cmd.destination = j["destination"].get<std::string>();
        } else {
            cmd.destination = "";
        }
        
        cmd.dry_run = j.value("dry_run", false);
        cmd.force = j.value("force", false);
        cmd.recursive = j.value("recursive", false);
        cmd.verbose = j.value("verbose", false);
        
        // Debug output to stderr
        std::cerr << "DEBUG: Command struct initialized:" << std::endl;
        std::cerr << "  action: " << cmd.action << std::endl;
        std::cerr << "  pattern: " << cmd.pattern << std::endl;
        std::cerr << "  source: " << cmd.source << std::endl;
        std::cerr << "  destination: " << cmd.destination << std::endl;
        std::cerr << "  recursive: " << (cmd.recursive ? "true" : "false") << std::endl;
        std::cerr << "  verbose: " << (cmd.verbose ? "true" : "false") << std::endl;
        
        if (cmd.verbose) {
            std::cerr << "Parsed command: action=" << cmd.action 
                      << ", pattern=" << cmd.pattern
                      << ", source=" << cmd.source
                      << ", destination=" << cmd.destination
                      << ", recursive=" << (cmd.recursive ? "true" : "false")
                      << std::endl;
        }
        
        // Validate command
        if (!actions::validate_command(cmd)) {
            std::cerr << "Invalid command" << std::endl;
            return 1;
        }
        
        // Execute command
        auto result = actions::execute_command(cmd);
        
        // Output ONLY the JSON result to stdout (no debug info)
        json output;
        output["success"] = result.success;
        output["operation"] = result.operation;
        output["message"] = result.message;
        output["files_scanned"] = result.files_scanned;
        output["files_matched"] = result.files_matched;
        output["files_affected"] = result.files_affected;
        output["start_time"] = std::to_string(result.start_time.time_since_epoch().count());
        output["end_time"] = std::to_string(result.end_time.time_since_epoch().count());
        
        // Add errors if any
        if (!result.errors.empty()) {
            output["errors"] = result.errors;
        }
        
        // Add error message if operation failed
        if (!result.success && !result.error_message.empty()) {
            output["error_message"] = result.error_message;
        }
        
        // Output ONLY the JSON to stdout
        std::cout << output.dump() << std::endl;
        
        return result.success ? 0 : 1;
        
    } catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
