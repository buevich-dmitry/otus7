#pragma once
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include <set>

namespace fs = boost::filesystem;

class FileFilterImpl;

class FileFilter {
public:
    FileFilter(
            std::vector<std::string> include_directories,
            std::vector<std::string> exclude_directories,
            int scan_level,
            size_t min_file_size,
            std::vector<std::string> file_masks);
    ~FileFilter();

    [[nodiscard]] std::set<fs::path> FilterFiles() const;

private:
    std::unique_ptr<FileFilterImpl> impl_;
};
