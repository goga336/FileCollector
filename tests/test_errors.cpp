#include <cstdint>
#include <gtest/gtest.h>
#include "file_collector.hpp"

//  Тесты для проверки обработки ошибок

// Тест для проверки CollectFile при попытке регистрации fileId, который уже был зарегистрирован
TEST(Errors, DuplicateCollect) {
    FileCollector obj;
    obj.CollectFile(111, 4);
    EXPECT_THROW(obj.CollectFile(111, 4), std::invalid_argument);
}

// Тест для проверки OnNewChunk при попытке передачи чанка с незарегистрированным fileId
TEST(Errors, UnknownFileOnNewChunk) {
    FileCollector obj;
    std::vector<uint8_t> buf = {1};
    EXPECT_THROW(obj.OnNewChunk(97, 0, std::move(buf)), std::out_of_range);
}

// Тест для проверки IsComplete при попытке запроса статуса файла с незарегистрированным fileId
TEST(Errors, UnknownFileOnIsComplete) {
    FileCollector obj;
    EXPECT_THROW(obj.IsComplete(96), std::out_of_range);
}

// Тест для проверки GetFile при попытке получения файла с незарегистрированным fileId
TEST(Errors, UnknownFileOnGetFile) {
    FileCollector obj;
    EXPECT_THROW(obj.GetFile(95), std::out_of_range);
}

// Тест для проверки RemoveFile при попытке удаления файла с незарегистрированным fileId
TEST(Errors, UnknownFileOnRemoveFile) {
    FileCollector obj;
    EXPECT_THROW(obj.RemoveFile(94), std::out_of_range);
}

// Тест для проверки OnNewChunk при попытке передачи чанка, выходящего за границы файла
TEST(Errors, ChunkOutOfBounds) {
    FileCollector obj;
    obj.CollectFile(1, 4);
    std::vector<uint8_t> big(5, 0);
    EXPECT_THROW(obj.OnNewChunk(1, 0, std::move(big)), std::out_of_range);  // 5 > 4
    std::vector<uint8_t> one = {1};
    EXPECT_THROW(obj.OnNewChunk(1, 5, std::move(one)), std::out_of_range);  // pos 5 > 4
}

// Тест для проверки OnNewChunk при попытке передачи чанка после завершения сборки файла
TEST(Errors, ChunkAfterComplete) {
    FileCollector obj;
    obj.CollectFile(1, 4);
    std::vector<uint8_t> buf = {1,2,3,4};
    obj.OnNewChunk(1, 0, std::move(buf));
    ASSERT_TRUE(obj.IsComplete(1));

    std::vector<uint8_t> duplicate = {1,2,3,4};
    EXPECT_NO_THROW(obj.OnNewChunk(1, 0, std::move(duplicate)));
    EXPECT_TRUE(obj.IsComplete(1));
}