#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
using namespace std;

class FileCollector{
    private:
    struct File{
        std::vector<uint8_t> data;
        std::vector<uint64_t> bitMask;
        size_t count = 0;
    };
    mutable std::mutex mx;
    std::unordered_map<uint32_t, std::shared_ptr<File>> files;
    public:
    FileCollector() = default;
    ~FileCollector() = default;
    FileCollector(const FileCollector&) = delete;
    FileCollector& operator=(const FileCollector&) = delete;
    void CollectFile(uint32_t field, size_t fileSize);
    void OnNewChunk(uint32_t field, size_t pos, std::vector<uint8_t> &&chunk);
    bool IsComplete(uint32_t field) const;

};