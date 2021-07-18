#define BOOST_TEST_MODULE test_reader

#include "hash.h"
#include "reader.h"
#include <iostream>
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_SUITE(test_reader)

static const std::string kTestFilePath = "test_block_reader_dir/test_file";

fs::path GetTestFilePath() {
    return fs::temp_directory_path() / kTestFilePath;
}

void CreateFile(const std::string& content) {
    fs::path path = GetTestFilePath();
    fs::create_directories(path.parent_path());
    fs::ofstream out{path};
    out << content;
}


void TestFileBlockReader(const std::string& content, size_t block_size) {
    CreateFile(content);
    FileBlockReader reader(GetTestFilePath(), block_size);
    for (size_t i = 0; i < content.size(); i += block_size) {
        assert(!reader.IsEnd());
        auto expected_block = content.substr(i, block_size);
        if (expected_block.size() < block_size) {
            expected_block += std::string(i + block_size - content.size(), 0);
        }
        BOOST_CHECK_EQUAL(expected_block, reader.ReadNextBlock());
    }
    assert(reader.IsEnd());
}

BOOST_AUTO_TEST_CASE(test_simple) {
    std::vector<std::string> contents{
        "short",
        "middle length text",
        "long1 long2 long3 long4 long5 long6 long7 long8 long9 long10 text",
        "text\nwith\neveral\nlines\n"};
    std::vector<size_t> block_sizes{1, 2, 3, 4, 5, 10, 50, 100, 1000};
    for (const auto& content : contents) {
        for (const auto block_size : block_sizes) {
            TestFileBlockReader(content, block_size);
        }
    }
}


}
