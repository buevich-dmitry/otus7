#pragma once
#include <memory>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

class FileBlockReaderImpl;

class FileBlockReader {
public:
    FileBlockReader(fs::path file_path, size_t block_size);
    ~FileBlockReader();

    std::string ReadNextBlock();
    bool IsEnd() const;

private:
    std::unique_ptr<FileBlockReaderImpl> impl_;
};



