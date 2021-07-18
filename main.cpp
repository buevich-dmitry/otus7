#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include "scanner.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main(int ac, char** av) {
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "produce help message")
            ("include-directories,i", po::value<std::vector<std::string>>()->required())
            ("exclude-directories,e", po::value<std::vector<std::string>>()->default_value({}, ""))
            ("scan-level,l", po::value<int>()->default_value(0))
            ("min-file-size,f", po::value<int>()->default_value(1))
            ("file-masks,m", po::value<std::vector<std::string>>()->default_value({".*"}, "\".*\""))
            ("block-size,b", po::value<int>()->required())
            ("hash-algorithm,a", po::value<std::string>()->default_value("md5"))
            ;

    po::variables_map vm;
    po::store(po::command_line_parser(ac, av).options(desc).run(), vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    po::notify(vm);

    Scanner scanner{
        vm["include-directories"].as<std::vector<std::string>>(),
        vm["exclude-directories"].as<std::vector<std::string>>(),
        vm["scan-level"].as<int>(),
        vm["min-file-size"].as<int>(),
        vm["file-masks"].as<std::vector<std::string>>(),
        vm["block-size"].as<int>(),
        vm["hash-algorithm"].as<std::string>()
    };

    const auto& file_groups = scanner.FindEqualFileGroups();
    for (const auto& file_group : file_groups) {
        for (const auto& file_name : file_group) {
            std::cout << file_name << "\n";
        }
        std::cout << "\n";
    }

    return 0;
}