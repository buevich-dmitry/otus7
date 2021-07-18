#include "hash.h"

#include <boost/uuid/sha1.hpp>
#include <boost/crc.hpp>
#include <map>
#include <openssl/md5.h>
#include <boost/functional/hash.hpp>
#include <iomanip>

using boost::uuids::detail::sha1;

HashValue GetHexHashRepresentation(const u_char* data, const size_t size) {
    std::stringstream ss;
    ss << std::hex;
    for (size_t i = 0; i < size; ++i) {
        ss << std::setw(2);
        ss << std::setfill('0');
        ss << static_cast<uint16_t>(data[i]);
    }
    return ss.str();
}

HashValue CalcSha1Hash(const std::string& block) {
    sha1 hash;
    uint32_t digest[5]{};
    hash.process_bytes(block.data(), block.size());
    hash.get_digest(digest);
    return GetHexHashRepresentation((u_char*)digest, sizeof(digest));
}

HashValue CalcHashCombine(const std::string& block) {
    size_t hash = boost::hash_range(block.begin(), block.end());
    return GetHexHashRepresentation((u_char*)&hash, sizeof(hash));
}

HashValue CalcCrc32Hash(const std::string& block) {
    boost::crc_32_type result;
    result.process_bytes(block.data(), block.size());
    const auto checksum = result.checksum();
    return GetHexHashRepresentation((u_char*)&checksum, sizeof(checksum));
}

HashValue CalcMd5Hash(const std::string& block) {
    std::vector<u_char> data(block.begin(), block.end());
    std::vector<u_char> result(MD5_DIGEST_LENGTH);
    MD5(data.data(), data.size(), result.data());
    return GetHexHashRepresentation(result.data(), result.size());
}

static const std::map<std::string, HashStrategy> kHashStrategies = {
        {"crc32", CalcCrc32Hash},
        {"hash_combine", CalcHashCombine},
        {"md5", CalcMd5Hash},
        {"sha1", CalcSha1Hash},
};

HashStrategy GetHashStrategy(const std::string& hash_algorithm) {
    return kHashStrategies.at(hash_algorithm);
}

std::vector<std::string> GetPossibleHashAlgorithms() {
    std::vector<std::string> result;
    std::transform(kHashStrategies.begin(), kHashStrategies.end(), std::back_inserter(result),
            [](const auto& kv) { return kv.first; });
    return result;
}
