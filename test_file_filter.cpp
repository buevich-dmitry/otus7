#define BOOST_TEST_MODULE test_file_filter

#include "file_filter.h"
#include <set>
#include <iostream>
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_SUITE(test_file_filter)

static const std::string kRootPath = "test_file_filter_dir";
static const size_t kFileSize = 10;

fs::path GetRootPath() {
    return fs::temp_directory_path() / kRootPath;
}

void CreateDirectories(const std::vector<std::string>& directories) {
    const fs::path root_path = GetRootPath();
    for (const auto& dir : directories) {
        assert(fs::path{dir}.is_relative());
        fs::create_directories(root_path / dir);
    }
}

void CreateFiles(const std::vector<std::string>& files) {
    const fs::path root_path = GetRootPath();
    for (const auto& file : files) {
        assert(fs::path(file).is_relative());
        fs::path path = root_path / file;
        fs::create_directories(path.parent_path());
        fs::ofstream out{path};
        out << std::string(kFileSize, '0');
    }
}

void ResetRootDirectory() {
    const fs::path root_path = GetRootPath();
    fs::remove_all(root_path);
    fs::create_directory(root_path);
    fs::current_path(root_path);
}

void TestFileFilter(
        const std::vector<std::string>& included_directories, const std::vector<std::string>& excluded_directories,
        int scan_level, size_t min_file_size, std::vector<std::string> file_masks,
        const std::vector<std::string>& all_files, const std::set<fs::path>& expected_files) {
    ResetRootDirectory();

    std::set<fs::path> absolute_expected_files;
    std::transform(expected_files.begin(), expected_files.end(),
                   std::inserter(absolute_expected_files, absolute_expected_files.end()),
                   [](const fs::path& path) { return fs::absolute(path); });

    CreateDirectories(included_directories);
    CreateDirectories(excluded_directories);
    CreateFiles(all_files);
    FileFilter file_filter(
            included_directories, excluded_directories, scan_level, min_file_size, std::move(file_masks));
    BOOST_CHECK(absolute_expected_files == file_filter.FilterFiles());
}

BOOST_AUTO_TEST_CASE(simple_test) {
    TestFileFilter({"."}, {}, 0, 0, {".*"}, {"a", "b"}, {"a", "b"});
    TestFileFilter({"."}, {}, 0, 0, {".*"}, {"d1/a", "d2/b"}, {});
}

BOOST_AUTO_TEST_CASE(test_scan_level) {
    TestFileFilter({"."}, {}, 1, 0, {".*"}, {"d1/a", "d2/b"}, {"d1/a", "d2/b"});
    TestFileFilter({"."}, {}, 1, 0, {".*"}, {"d1/a", "d2/b", "c"}, {"d1/a", "d2/b", "c"});
    TestFileFilter({"."}, {}, 1, 0, {".*"}, {"d1/a", "d2/b", "d1/dd1/c"}, {"d1/a", "d2/b"});

    TestFileFilter({"."}, {}, 2, 0, {".*"}, {"d1/a", "d2/b", "d1/dd1/c"}, {"d1/a", "d2/b", "d1/dd1/c"});
}

BOOST_AUTO_TEST_CASE(test_min_file_size) {
    TestFileFilter({"."}, {}, 0, kFileSize + 1, {".*"}, {"a", "b"}, {});
}

BOOST_AUTO_TEST_CASE(test_include_exclude_directories) {
    TestFileFilter({"d1"}, {}, 1, 0, {".*"}, {"d1/a", "d2/b", "c"}, {"d1/a"});
    TestFileFilter({"d1", "d2"}, {}, 1, 0, {".*"}, {"d1/a", "d2/b", "c"}, {"d1/a", "d2/b"});
    TestFileFilter({"d1"}, {"d1/dd1"}, 1, 0, {".*"}, {"d1/a", "d1/dd1/b", "d1/dd2/c", "d"}, {"d1/a", "d1/dd2/c"});
}

BOOST_AUTO_TEST_CASE(test_file_masks) {
    TestFileFilter({"."}, {}, 1, 0, {"a"}, {"a", "b", "d1/a", "d1/b"}, {"a", "d1/a"});
    TestFileFilter({"."}, {}, 1, 0, {"\\d+"}, {"a", "1", "d1/a", "d1/22"}, {"1", "d1/22"});
}

}
