#include <gtest/gtest.h>
#include "file_collector.hpp"

// Тесты для проверки логики сборки файла из чанков

// Тест для проверки OnNewChunk, проверяет сборку файла из пересекающихся чанков, приходящих в произвольном порядке
TEST(Chunks, IntersectingExampleFromTask) {
    FileCollector obj;
    obj.CollectFile(1, 10);

    std::vector<uint8_t> a = {'A','B','C','D'};
    obj.OnNewChunk(1, 0, std::move(a));
    EXPECT_FALSE(obj.IsComplete(1));

    std::vector<uint8_t> b = {'C','D','E','F'};
    obj.OnNewChunk(1, 2, std::move(b));
    EXPECT_FALSE(obj.IsComplete(1));

    std::vector<uint8_t> c = {'G','H','I','J'};
    obj.OnNewChunk(1, 6, std::move(c));
    EXPECT_TRUE(obj.IsComplete(1));

    auto f = obj.GetFile(1);
    ASSERT_NE(f, nullptr);
    const char* expected = "ABCDEFGHIJ";
    for (size_t i = 0; i < 10; ++i) {
        EXPECT_EQ((*f)[i], static_cast<uint8_t>(expected[i])) << "at index " << i;
    }
}


// Тест для проверки OnNewChunk, проверяет сборку файла из чанков, приходящих в обратном порядке
TEST(Chunks, ReverseOrder) {
    FileCollector obj;
    obj.CollectFile(1, 6);

    std::vector<uint8_t> chunk1= {4,5,6};
    obj.OnNewChunk(1, 3, std::move(chunk1));
    EXPECT_FALSE(obj.IsComplete(1));

    std::vector<uint8_t> chunk2 = {1,2,3};
    obj.OnNewChunk(1, 0, std::move(chunk2));
    EXPECT_TRUE(obj.IsComplete(1));

    auto f = obj.GetFile(1);
    ASSERT_NE(f, nullptr);
    for (size_t i = 0; i < 6; ++i) {
        EXPECT_EQ((*f)[i], static_cast<uint8_t>(i + 1)) << "at index " << i;
    }
}

// Тест для проверки OnNewChunk, проверяет сборку файла из чанков, дублирующих друг друга
TEST(Chunks, DuplicateChunk) {
    FileCollector obj;
    obj.CollectFile(1, 4);

    std::vector<uint8_t> c = {1,2,3,4};
    obj.OnNewChunk(1, 0, std::move(c));
    ASSERT_TRUE(obj.IsComplete(1));

    std::vector<uint8_t> dup = {1,2,3,4};
    obj.OnNewChunk(1, 0, std::move(dup));
    EXPECT_TRUE(obj.IsComplete(1));

    auto f = obj.GetFile(1);
    ASSERT_NE(f, nullptr);
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_EQ((*f)[i], static_cast<uint8_t>(i + 1)) << "at index " << i;
    }
}

// Тест для проверки OnNewChunk, проверяет сборку файла при передаче пустого чанка
TEST(Chunks, EmptyChunk) {
    FileCollector obj;
    obj.CollectFile(1, 5);

    std::vector<uint8_t> empty;
    obj.OnNewChunk(1, 0, std::move(empty));

    EXPECT_FALSE(obj.IsComplete(1));
    EXPECT_EQ(obj.GetFile(1), nullptr);
}

// Тест для проверки GetFile, проверяет корректность обработки попытки получения несобранного файла
TEST(Chunks, GetFileNull) {
    FileCollector obj;
    obj.CollectFile(1, 4);

    EXPECT_EQ(obj.GetFile(1), nullptr);

    std::vector<uint8_t> partial = {1, 2};
    obj.OnNewChunk(1, 0, std::move(partial));

    EXPECT_FALSE(obj.IsComplete(1));
    EXPECT_EQ(obj.GetFile(1), nullptr);
}

// Тест для проверки RemoveFile, проверяет корректность удаления файла
TEST(Chunks, RemoveFile) {
    FileCollector obj;
    obj.CollectFile(1, 4);

    EXPECT_NO_THROW(obj.RemoveFile(1));

    EXPECT_THROW(obj.IsComplete(1), std::out_of_range);
    EXPECT_THROW(obj.GetFile(1), std::out_of_range);
    EXPECT_THROW(obj.RemoveFile(1), std::out_of_range);
}