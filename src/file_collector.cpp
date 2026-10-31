#include "file_collector.hpp"
#include <cstddef>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <new>
#include <cstring>
#include <utility>

void FileCollector::CollectFile(uint32_t fileId, size_t fileSize){
    
    std::shared_ptr<File> file;
    file = std::make_shared<File>();
    
    // выделение буфера
    try{
        file->data.assign(fileSize, 0); //выделение буфера и заполнение его нулями
        file->bitMask.assign(fileSize, false); // выделение маски
    } catch(const std::bad_alloc &){
        throw std::runtime_error("Out of memory for file");
    }
    catch(const std::length_error &){
        throw std::runtime_error("The size of file is too large");
    }
    

    std::lock_guard<std::mutex> lock(filesMx); // захват мьютекса таблицы  files
    if(files.find(fileId) != files.end()){ // проверка на дубликата
        throw std::invalid_argument("This fileId is exist");
    }
    files.emplace(fileId, std::move(file)); // регистрация файла
}

std::shared_ptr<FileCollector::File> FileCollector::FindFile(uint32_t fileId) const {
    std::lock_guard<std::mutex> lock(filesMx);
    auto it = files.find(fileId);
    if(it == files.end()){
        throw std::out_of_range("This fileId is unknown");
    }
    return it->second;
}

void FileCollector::OnNewChunk(uint32_t fileId, size_t pos, std::vector<uint8_t> &&chunk){
    std::shared_ptr<File> file = FindFile(fileId);
    std::lock_guard<std::mutex> lock(file->mx);
    
    const size_t fileSize = file->data.size();
    if(pos > fileSize || (fileSize - pos) < chunk.size()){ // проверка границ
        throw std::out_of_range("This chunk outside the boundaries");
    }
    if(chunk.empty()){ // проверка на пустой чанк
        return;
    }
    if(file->count == fileSize){ // проверка на уже собранный файл
        return;
    }
    std::memcpy(file->data.data() + pos, chunk.data(), chunk.size()); // копирование чанка сразу на его место в файле
    // отмечаем новые байты в маске
    size_t countAdded = 0;
    for(size_t i = 0; i < chunk.size(); i++){
        if(file->bitMask[i + pos] != true){
            file->bitMask[i + pos] = true;
            countAdded+=1;
        }
    }
    file->count += countAdded;
}

bool FileCollector::IsComplete(uint32_t fileId) const{
    std::shared_ptr<File> file = FindFile(fileId);
    std::lock_guard<std::mutex> lock(file->mx);
    
    if(file->count == file->data.size()){
        return true;
    }
    return false;

}

std::shared_ptr<const std::vector<uint8_t>> FileCollector::GetFile(uint32_t fileId) const{
    std::shared_ptr<File> file = FindFile(fileId);
    std::lock_guard<std::mutex> lock(file->mx);
    
    if(file->count != file->data.size()){
        return nullptr;
    }
    return std::shared_ptr<const std::vector<uint8_t>>(file, &file->data); // возврат умного указателя на буфер
}

void FileCollector::RemoveFile(uint32_t fileId){
    std::lock_guard<std::mutex> lock(filesMx);
    auto it = files.find(fileId);

    if(it == files.end()){
        throw std::out_of_range("This fileId is unknown");
    }
    files.erase(it);
}

bool FileCollector::ReadRange(uint32_t fileId, size_t pos, size_t len, uint8_t *outBuf) const{
    if(outBuf == nullptr){ // проверка аргмента outBuf на nullptr
        throw std::invalid_argument("outBuf is null");
    }
    std::shared_ptr<File> file = FindFile(fileId);
    std::lock_guard<std::mutex> lock(file->mx); // захват мьютекса

    const size_t fileSize = file->data.size();
    if(pos > fileSize || (fileSize - pos) < len){ // проверка границ
        throw std::out_of_range("This range outside the boundaries");
    }
    if(len == 0){
        return true;
    }
    if(file->count != fileSize){ // проверка на собранность файла
        for(size_t i = 0; i < len; i++){
            if(file->bitMask[i + pos] == 0){
                return false;
            }
        } 
    }
    
    std::memcpy(outBuf, file->data.data() + pos,  len);
    return true;


}


