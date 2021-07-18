#include "scanner.h"
#include "hash.h"
#include "reader.h"
#include "file_filter.h"
#include <unordered_set>
#include <iostream>
#include <boost/format.hpp>
#include <map>



class FileTrie {
public:
    FileTrie(size_t block_size, HashStrategy hash_strategy)
        : block_size_(block_size), head_(std::make_shared<Node>()), hash_strategy_(hash_strategy) {
    }

    void AddFile(const fs::path file_path) {
        assert(files_to_handle_.empty());
        AddFile(head_, std::make_shared<FileData>(file_path, block_size_));
        HandleSavedFiles();
        assert(files_to_handle_.empty());
    }

    std::vector<std::vector<fs::path>> GetEqualFileGroups() {
        assert(files_to_handle_.empty());
        std::vector<std::vector<fs::path>> result;
        FindEqualFileGroups(head_, result);
        return result;
    }

private:
    struct FileData {
        FileData(fs::path file_path, size_t block_size)
                : file_path(file_path), file_block_reader(file_path, block_size) {
        }

        fs::path file_path;
        FileBlockReader file_block_reader;
    };

    struct Node {
        std::unordered_map<HashValue, std::shared_ptr<Node>> next_nodes;
        std::set<std::shared_ptr<FileData>> file_data_set;
    };

    void AddFile(std::shared_ptr<Node> node, const std::shared_ptr<FileData>& file_data) {
        assert(file_data != nullptr);
        while (!file_data->file_block_reader.IsEnd() &&
                (!node->next_nodes.empty() || !node->file_data_set.empty())) {
            for (auto iter = node->file_data_set.begin(); iter != node->file_data_set.end();) {
                const auto node_file_data = *iter;
                if (node_file_data->file_block_reader.IsEnd()) {
                    // this file data is not going to be moved anywhere
                    ++iter;
                    continue;
                }
                files_to_handle_.emplace(node, node_file_data);
                iter = node->file_data_set.erase(iter);
            }
            HashValue hash = hash_strategy_(file_data->file_block_reader.ReadNextBlock());
            auto& next_node = node->next_nodes[hash];
            if (next_node == nullptr) {
                next_node = std::make_shared<Node>();
            }
            node = next_node;
        }
        // reading blocks is not needed, place current file here
        node->file_data_set.insert(file_data);
    }

    void HandleSavedFiles() {
        while (!files_to_handle_.empty()) {
            auto [node, file_data] = files_to_handle_.top();
            files_to_handle_.pop();
            AddFile(node, file_data);
        }
    }

    void FindEqualFileGroups(const std::shared_ptr<Node>& node, std::vector<std::vector<fs::path>>& result) {
        assert(node != nullptr);
        if (node->file_data_set.size() > 1) {
            std::vector<fs::path> equal_group;
            for (const auto& file_data : node->file_data_set) {
                assert(file_data->file_block_reader.IsEnd());
                equal_group.push_back(file_data->file_path);
            }
            result.push_back(std::move(equal_group));
        }
        for (const auto& [_, next_node] : node->next_nodes) {
            FindEqualFileGroups(next_node, result);
        }
    }

    size_t block_size_;
    std::shared_ptr<Node> head_;
    HashStrategy hash_strategy_;
    std::stack<std::pair<std::shared_ptr<Node>, std::shared_ptr<FileData>>> files_to_handle_;
};


class ScannerImpl {
public:
    ScannerImpl(
            std::vector<std::string> include_directories,
            std::vector<std::string> exclude_directories,
            int scan_level,
            int min_file_size,
            std::vector<std::string> file_masks,
            int block_size,
            std::string hash_algorithm)
            : file_filter_(
                    std::move(include_directories),
                    std::move(exclude_directories),
                    scan_level,
                    min_file_size,
                    std::move(file_masks))
            , block_size_(block_size)
            , hash_algorithm_(std::move(hash_algorithm)) {
        std::ignore = std::make_tuple(block_size_);
    }

    [[nodiscard]] std::vector<std::vector<fs::path>> FindEqualFileGroups() const {
        FileTrie file_trie(block_size_, GetHashStrategy(hash_algorithm_));
        for (const auto& file_path : file_filter_.FilterFiles()) {
            file_trie.AddFile(file_path);
        }
        return file_trie.GetEqualFileGroups();
    }

private:
    FileFilter file_filter_;
    int block_size_;
    std::string hash_algorithm_;
};

Scanner::Scanner(
        std::vector<std::string> include_directories,
        std::vector<std::string> exclude_directories,
        int scan_level,
        int min_file_size,
        std::vector<std::string> file_masks,
        int block_size,
        std::string hash_algorithm)
        : impl_(std::make_unique<ScannerImpl>(
                std::move(include_directories),
                std::move(exclude_directories),
                scan_level,
                min_file_size,
                std::move(file_masks),
                block_size,
                std::move(hash_algorithm))) {
}

Scanner::~Scanner() = default;

std::vector<std::vector<fs::path>> Scanner::FindEqualFileGroups() const {
    return impl_->FindEqualFileGroups();
}
