#include <iostream>
#include <cassert>
#include <filesystem>
#include <string>
#include <vector>
#include "../cpp_backend/utils.hpp"
#include "../cpp_backend/actions.hpp"

// Simple test framework
#define TEST(name) void test_##name()
#define ASSERT(condition) assert(condition)
#define ASSERT_EQ(a, b) assert((a) == (b))
#define ASSERT_NE(a, b) assert((a) != (b))
#define ASSERT_TRUE(condition) assert(condition)
#define ASSERT_FALSE(condition) assert(!(condition))

// Test utility functions
TEST(expand_path) {
    std::cout << "Testing expand_path..." << std::endl;
    
    // Test absolute path (should remain unchanged)
    auto abs_path = utils::expand_path("/absolute/path");
    ASSERT_EQ(abs_path.string(), "/absolute/path");
    
    // Test relative path (should be resolved)
    auto rel_path = utils::expand_path("relative/path");
    ASSERT_TRUE(rel_path.is_absolute());
    
    std::cout << "✓ expand_path tests passed" << std::endl;
}

TEST(is_safe_directory) {
    std::cout << "Testing is_safe_directory..." << std::endl;
    
    // Test system directories (should be unsafe)
    ASSERT_FALSE(utils::is_safe_directory("/"));
    ASSERT_FALSE(utils::is_safe_directory("/bin"));
    ASSERT_FALSE(utils::is_safe_directory("/usr"));
    
    // Test user directories (should be safe)
    ASSERT_TRUE(utils::is_safe_directory("/home"));
    ASSERT_TRUE(utils::is_safe_directory("/tmp"));
    
    std::cout << "✓ is_safe_directory tests passed" << std::endl;
}

TEST(scan_directory) {
    std::cout << "Testing scan_directory..." << std::endl;
    
    // Create temporary test directory
    std::filesystem::path test_dir = "/tmp/smartfilecmd_test";
    std::filesystem::create_directories(test_dir);
    
    // Create some test files
    std::filesystem::path file1 = test_dir / "test1.txt";
    std::filesystem::path file2 = test_dir / "test2.jpg";
    std::filesystem::path file3 = test_dir / "test3.png";
    
    std::ofstream(file1).close();
    std::ofstream(file2).close();
    std::ofstream(file3).close();
    
    // Test scanning all files
    auto all_files = utils::scan_directory(test_dir);
    ASSERT_EQ(all_files.size(), 3);
    
    // Test scanning with extension filter
    auto txt_files = utils::scan_directory(test_dir, ".txt");
    ASSERT_EQ(txt_files.size(), 1);
    ASSERT_EQ(txt_files[0].extension(), ".txt");
    
    auto jpg_files = utils::scan_directory(test_dir, ".jpg");
    ASSERT_EQ(jpg_files.size(), 1);
    ASSERT_EQ(jpg_files[0].extension(), ".jpg");
    
    // Cleanup
    std::filesystem::remove_all(test_dir);
    
    std::cout << "✓ scan_directory tests passed" << std::endl;
}

TEST(matches_pattern) {
    std::cout << "Testing matches_pattern..." << std::endl;
    
    std::filesystem::path file_path = "test_file.txt";
    
    // Test exact match
    ASSERT_TRUE(utils::matches_pattern(file_path, "test_file.txt"));
    
    // Test extension match
    ASSERT_TRUE(utils::matches_pattern(file_path, "*.txt"));
    
    // Test no match
    ASSERT_FALSE(utils::matches_pattern(file_path, "*.jpg"));
    
    // Test wildcard pattern
    ASSERT_TRUE(utils::matches_pattern(file_path, "test_*"));
    
    std::cout << "✓ matches_pattern tests passed" << std::endl;
}

TEST(validate_command) {
    std::cout << "Testing validate_command..." << std::endl;
    
    // Test valid commands
    actions::Command valid_cmd1 = {"delete", ".png", "~/Downloads", "", false, false};
    ASSERT_TRUE(actions::validate_command(valid_cmd1));
    
    actions::Command valid_cmd2 = {"move", ".jpg", "~/Downloads", "~/Pictures", false, false};
    ASSERT_TRUE(actions::validate_command(valid_cmd2));
    
    // Test invalid commands
    actions::Command invalid_cmd1 = {"", ".png", "~/Downloads", "", false, false};
    ASSERT_FALSE(actions::validate_command(invalid_cmd1));
    
    actions::Command invalid_cmd2 = {"move", ".jpg", "~/Downloads", "", false, false};
    ASSERT_FALSE(actions::validate_command(invalid_cmd2));
    
    std::cout << "✓ validate_command tests passed" << std::endl;
}

TEST(command_to_string) {
    std::cout << "Testing command_to_string..." << std::endl;
    
    actions::Command cmd = {"move", ".jpg", "~/Downloads", "~/Pictures", true, false};
    std::string cmd_str = actions::command_to_string(cmd);
    
    ASSERT_TRUE(cmd_str.find("move") != std::string::npos);
    ASSERT_TRUE(cmd_str.find(".jpg") != std::string::npos);
    ASSERT_TRUE(cmd_str.find("DRY RUN") != std::string::npos);
    
    std::cout << "✓ command_to_string tests passed" << std::endl;
}

TEST(create_folder_dry_run) {
    std::cout << "Testing create_folder with dry_run..." << std::endl;
    
    actions::Command cmd = {"create_folder", "", "", "/tmp/test_folder", true, false};
    
    auto result = actions::create_folder(cmd);
    
    ASSERT_TRUE(result.success);
    ASSERT_TRUE(result.message.find("DRY RUN") != std::string::npos);
    ASSERT_EQ(result.files_affected, 0);
    
    std::cout << "✓ create_folder dry_run tests passed" << std::endl;
}

int main() {
    std::cout << "🧪 Running C++ Backend Tests..." << std::endl;
    std::cout << "==================================" << std::endl;
    
    try {
        test_expand_path();
        test_is_safe_directory();
        test_scan_directory();
        test_matches_pattern();
        test_validate_command();
        test_command_to_string();
        test_create_folder_dry_run();
        
        std::cout << "==================================" << std::endl;
        std::cout << "🎉 All tests passed!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Test failed with unknown error" << std::endl;
        return 1;
    }
}
