#pragma once
#include <memory>
#include <boost/filesystem.hpp>

using HashValue = std::string;
using HashStrategy = std::function<HashValue(std::string)>;

HashStrategy GetHashStrategy(const std::string& hash_algorithm);

std::vector<std::string> GetPossibleHashAlgorithms();
