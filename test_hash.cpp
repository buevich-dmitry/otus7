#define BOOST_TEST_MODULE test_hash

#include "hash.h"
#include <iostream>
#include <iomanip>
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_SUITE(test_hash)

void TestHashAlgorithm(const std::string& hash_algorithm, const std::string& block,
                       const std::string& expected_hash_hex, const size_t expected_result_size) {
    const auto result = GetHashStrategy(hash_algorithm)(block);
    BOOST_CHECK_EQUAL(expected_hash_hex, result);
    BOOST_CHECK_EQUAL(expected_result_size, result.size());
}

BOOST_AUTO_TEST_CASE(test_possible_hash_algorithms) {
    BOOST_CHECK(GetPossibleHashAlgorithms() == std::vector<std::string>({"crc32", "hash_combine", "md5", "sha1"}));
}

BOOST_AUTO_TEST_CASE(test_hash_algorithms) {
    const std::string block = "some text";
    TestHashAlgorithm("crc32", block, "babdba4f", 8);
    TestHashAlgorithm("hash_combine", block, "501f002eee0fa5c7", 16);
    TestHashAlgorithm("md5", block, "552e21cd4cd9918678e3c1a0df491bc3", 32);
    TestHashAlgorithm("sha1", block, "c763aa3754d99873e16232471e7c05a077da2e63", 40);
}

}
