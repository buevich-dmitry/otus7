#include "reader.h"

class FileBlockReaderImpl {
public:
    FileBlockReaderImpl(fs::path file_path, size_t block_size)
            : file_path_(std::move(file_path))
            , block_size_(block_size) {
        assert(fs::exists(file_path_));
        assert(fs::is_regular_file(file_path_));
    }

    std::string ReadNextBlock() {
        assert(!IsEnd());
        if (!ifstream_) {
            ifstream_.emplace(file_path_);
            assert(ifstream_->good());
        }
        std::string block(block_size_, 0);
        for (size_t i = 0; i < block_size_; ++i) {
            ifstream_->get(block[i]);
            if (ifstream_->eof()) {
                break;
            }
        }
        if (!ifstream_->eof() && ifstream_->get() != fs::ofstream::traits_type::eof()) {
            ifstream_->unget();
        }
        return block;
    }

    bool IsEnd() const {
        return ifstream_.has_value() && ifstream_->eof();
    }

private:
    fs::path file_path_;
    size_t block_size_;
    std::optional<fs::ifstream> ifstream_{};
};

FileBlockReader::FileBlockReader(fs::path file_path, size_t block_size)
        : impl_(std::make_unique<FileBlockReaderImpl>(std::move(file_path), block_size)) {
}

FileBlockReader::~FileBlockReader() = default;

std::string FileBlockReader::ReadNextBlock() {
    return impl_->ReadNextBlock();
}

bool FileBlockReader::IsEnd() const {
    return impl_->IsEnd();
}

