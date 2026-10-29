#include "../inc/file_collector.h"
#include <cstddef>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <cstring>

void FileCollector::CollectFile(uint32_t field, size_t fileSize){
    std::lock_guard<std::mutex> lock(mx);
    if(files.find(field) != files.end()){
        throw std::invalid_argument("This field is exist");
    }
    std::shared_ptr<File> file = std::make_shared<File>();
    file->data.assign(fileSize, 0);
    file->mask.assign(fileSize, false);
    files.emplace(field, std::move(file));
}

void FileCollector::OnNewChunk(uint32_t field, size_t pos, std::vector<uint8_t> &&chunk){
    std::lock_guard<std::mutex> lock(mx);
    
    if(files.find(field) == files.end()){
        throw std::out_of_range("This field is unknown");
    }
    std::shared_ptr<File> file = files.at(field);
    const size_t fileSize = file->data.size();
    if(pos > fileSize || (fileSize - pos) < chunk.size()){
        throw std::out_of_range("This chunk outside the boundaries");
    }
    if(chunk.empty()){
        return;
    }
    std::memcpy(file->data.data() + pos, chunk.data(), chunk.size());
    size_t countAded = 0;
    for(size_t i = 0; i < chunk.size(); i++){
        if(file->mask[i + pos] != true){
            file->mask[i + pos] = true;
            countAded+=1;
        }
    }
    file->count += countAded;
}

