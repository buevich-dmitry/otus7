#define BOOST_TEST_MODULE test_scanner

#include "scanner.h"
#include <set>
#include <unordered_set>
#include <iostream>
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_SUITE(test_scanner)

static const std::string kRootPath = "test_scanner_dir";

fs::path GetRootPath() {
    return fs::temp_directory_path() / kRootPath;
}

void CreateFile(const std::string& file_name, const std::string& content) {
    assert(fs::path{file_name}.is_relative());
    fs::path path = GetRootPath() / file_name;
    fs::ofstream out{path};
    out << content;
}

void ResetRootDirectory() {
    const fs::path root_path = GetRootPath();
    fs::remove_all(root_path);
    fs::create_directory(root_path);
    fs::current_path(root_path);
}

std::string JoinPaths(const std::vector<fs::path>& paths) {
    std::stringstream ss;
    bool first = true;
    for (const auto& path : paths) {
        if (!first) {
            ss << '\n';
        }
        ss << path;
    }
    return ss.str();
}

std::vector<std::vector<fs::path>> CanonizeFileGroups(std::vector<std::vector<fs::path>> file_groups) {
    for (auto& group : file_groups) {
        std::sort(group.begin(), group.end());
    }
    std::sort(file_groups.begin(), file_groups.end(), [](const auto& lhs, const auto& rhs) {
           return JoinPaths(lhs) < JoinPaths(rhs);
    });
    return file_groups;
}

void TestScanner(std::unordered_map<std::string, std::string> file_name_to_file_content) {
    ResetRootDirectory();
    std::unordered_map<std::string, std::vector<fs::path>> content_to_file_names;
    for (const auto& [name, content] : file_name_to_file_content) {
        CreateFile(name, content);
        content_to_file_names[content].push_back(GetRootPath() / name);
    }
    std::vector<std::vector<fs::path>> expected_file_groups;
    for (const auto& [_, file_names] : content_to_file_names) {
        if (file_names.size() > 1) {
            expected_file_groups.push_back(file_names);
        }
    }

    Scanner scanner{{"."}, {}, 0, 0, {".*"}, 1, "sha1"};
    BOOST_CHECK(CanonizeFileGroups(expected_file_groups) == CanonizeFileGroups(scanner.FindEqualFileGroups()));
}

BOOST_AUTO_TEST_CASE(simple_test) {
    TestScanner({{"a", "1"}});
    TestScanner({{"a", "1"}, {"b", "1"}});
    TestScanner({{"a", "1"}, {"b", "1"}, {"c", "1"}});
    TestScanner({{"a", "1"}, {"b", "1"}, {"c", "1"}, {"d", "2"}, {"e", "2"}});
    TestScanner({{"a", "11"}, {"b", "11"}, {"c", "12"}, {"d", "121"}, {"e", "121"}, {"f", "122"}});
    TestScanner({{"a", "11"}, {"b", "11"}, {"c", "121"}, {"d", "121"}, {"e", "121"}, {"f", "122"}});
    TestScanner({{"a", "11"}, {"b", "11"}, {"c", "121"}, {"d", "121"}, {"e", "121"}, {"f", "222"}});
    TestScanner({{"a", "11"}, {"b", "11"}, {"c", "121"}, {"d", "121"}, {"e", "121"}, {"f", "222"}, {"g", "222"}});
}

}
