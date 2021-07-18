#pragma once
#include <vector>
#include <string>
#include <memory>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

class ScannerImpl;

class Scanner {
public:
    Scanner(
            std::vector<std::string> include_directories,
            std::vector<std::string> exclude_directories,
            int scan_level,
            int min_file_size,
            std::vector<std::string> file_masks,
            int block_size,
            std::string hash_algorithm);
    ~Scanner();

    [[nodiscard]] std::vector<std::vector<fs::path>> FindEqualFileGroups() const;

private:
    std::unique_ptr<ScannerImpl> impl_;
};
