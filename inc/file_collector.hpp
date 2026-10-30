#ifndef MYPROJECT_FILE_COLLECTOR_HPP
#define MYPROJECT_FILE_COLLECTOR_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

class FileCollector{
    private:
    struct File{
        mutable std::mutex mx;
        std::vector<uint8_t> data;
        std::vector<bool> bitMask;
        size_t count = 0;
    };
    mutable std::mutex filesMx;
    std::unordered_map<uint32_t, std::shared_ptr<File>> files;
    std::shared_ptr<File> FindFile(uint32_t fileId) const;
    public:
    FileCollector() = default;
    ~FileCollector() = default;
    FileCollector(const FileCollector&) = delete;
    FileCollector& operator=(const FileCollector&) = delete;
    void CollectFile(uint32_t fileId, size_t fileSize);
    void OnNewChunk(uint32_t fileId, size_t pos, std::vector<uint8_t> &&chunk);
    bool IsComplete(uint32_t fileId) const;
    std::shared_ptr<const std::vector<uint8_t>> GetFile(uint32_t fileId) const;
    void RemoveFile(uint32_t fileId);
    bool ReadRange(uint32_t fileId, size_t pos, size_t len, uint8_t* out) const;
    

};
#endif 