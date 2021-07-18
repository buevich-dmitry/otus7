#include "file_filter.h"
#include <boost/regex.hpp>
#include <iostream>

std::set<fs::path> ConvertStringsToPaths(std::vector<std::string> paths) {
    std::set<fs::path> result{};
    std::transform(std::make_move_iterator(paths.begin()), std::make_move_iterator(paths.end()),
                   std::inserter(result, result.end()),
                   [](std::string path) { return fs::canonical(fs::absolute(path)); });
    return result;
}

std::vector<boost::regex> ConvertToRegex(std::vector<std::string> strings) {
    std::vector<boost::regex> result;
    std::transform(std::make_move_iterator(strings.begin()), std::make_move_iterator(strings.end()),
                   std::inserter(result, result.end()), [](std::string str) { return boost::regex{std::move(str)}; });
    return result;
}


class FileFilterImpl {
public:
    FileFilterImpl(
            std::vector<std::string> include_directories,
            std::vector<std::string> exclude_directories,
            int scan_level,
            size_t min_file_size,
            std::vector<std::string> file_masks)
            : include_directories_(ConvertStringsToPaths(std::move(include_directories)))
            , exclude_directories_(ConvertStringsToPaths(std::move(exclude_directories)))
            , scan_level_(scan_level)
            , min_file_size_(min_file_size)
            , file_masks_(ConvertToRegex(std::move(file_masks))) {
        CheckDirectories(include_directories_);
        CheckDirectories(exclude_directories_);
    }

    [[nodiscard]] std::set<fs::path> FilterFiles() const {
        std::set<fs::path> result{};
        for (const auto& directory : include_directories_) {
            result.merge(FilterFiles(directory, scan_level_));
        }
        return result;
    }

private:
    void CheckDirectories(const std::set<fs::path>& directories) const {
        for (const auto& directory : directories) {
            assert(fs::exists(directory));
            assert(fs::is_directory(directory));
        }
    }

    [[nodiscard]] std::set<fs::path> FilterFiles(const fs::path& directory, int scan_level) const {
        assert(fs::is_directory(directory));
        if (scan_level < 0) {
            return {};
        }
        std::set<fs::path> result;
        for (fs::path path : fs::directory_iterator(directory)) {
            path = fs::canonical(path);
            assert(path.is_absolute());
            if (fs::is_directory(path) && !exclude_directories_.count(path)) {
                result.merge(FilterFiles(path, scan_level - 1));
            }
            if (fs::is_regular_file(path) && CheckFile(path)) {
                result.insert(path);
            }
        }
        return result;
    }

    [[nodiscard]] bool CheckFile(const fs::path& file) const {
        assert(fs::is_regular_file(file));
        for (const auto& file_mask : file_masks_) {
            if (boost::regex_match(file.filename().string(), file_mask) && fs::file_size(file) >= min_file_size_) {
                return true;
            }
        }
        return false;
    }

    std::set<fs::path> include_directories_;
    std::set<fs::path> exclude_directories_;
    int scan_level_;
    size_t min_file_size_;
    std::vector<boost::regex> file_masks_;
};

FileFilter::FileFilter(
        std::vector<std::string> include_directories,
        std::vector<std::string> exclude_directories,
        int scan_level,
        size_t min_file_size,
        std::vector<std::string> file_masks)
        : impl_(std::make_unique<FileFilterImpl>(
                std::move(include_directories),
                std::move(exclude_directories),
                scan_level,
                min_file_size,
                std::move(file_masks))){
}

FileFilter::~FileFilter() = default;

std::set<fs::path> FileFilter::FilterFiles() const {
    return impl_->FilterFiles();
}
