#include <gtest/gtest.h>
#include "file_collector.hpp"

// Тесты для проверки функции ReadRange

// Тест проверяет корректность возвращаемых данных на диапазоне pos, pos+len
TEST(ReadRange, CompleteRange) {
    FileCollector obj;
    obj.CollectFile(11, 10);
    std::vector<uint8_t> c = {0,1,2,3,4,5,6,7,8,9};
    obj.OnNewChunk(11, 0, std::move(c));

    uint8_t buf[4] = {};
    EXPECT_TRUE(obj.ReadRange(11, 2, 4, buf));
    EXPECT_EQ(buf[0], 2);
    EXPECT_EQ(buf[1], 3);
    EXPECT_EQ(buf[2], 4);
    EXPECT_EQ(buf[3], 5);
}

// Тест проверяет корректность работы функции при наличии разрыва в запрошенном диапазоне
TEST(ReadRange, PartialFileHoleReturnsFalse) {
    FileCollector obj;
    obj.CollectFile(1, 10);
    std::vector<uint8_t> c = {0,1,2};
    obj.OnNewChunk(1, 0, std::move(c));

    uint8_t buf[4] = {};
    EXPECT_FALSE(obj.ReadRange(1, 0, 4, buf));
}

// Тест проверяет корректность работы функции при запросе получения буфера с длиной 0
TEST(ReadRange, ZeroLenReturnsTrue) {
    FileCollector obj;
    obj.CollectFile(1, 4);
    uint8_t buf[1];
    EXPECT_TRUE(obj.ReadRange(1, 0, 0, buf));
}

// Тест проверяет корректность работы функции при запросе получения буфера c передаваемым нулевым указателем
TEST(ReadRange, NullBufThrows) {
    FileCollector obj;
    obj.CollectFile(1, 4);
    EXPECT_THROW(obj.ReadRange(1, 0, 4, nullptr), std::invalid_argument);
}

// Тест проверяет корректность работы функции при запросе диапазона, выходящего за пределы файла
TEST(ReadRange, OutOfBoundsThrows) {
    FileCollector obj;
    obj.CollectFile(1, 4);
    uint8_t buf[10];
    EXPECT_THROW(obj.ReadRange(1, 0, 5, buf), std::out_of_range);
    EXPECT_THROW(obj.ReadRange(1, 5, 1, buf), std::out_of_range);
}

// Тест проверяет корректность работы функции при запросе получения буфера с неизвестным fileId
TEST(ReadRange, UnknownFileThrows) {
    FileCollector obj;
    uint8_t buf[10];
    EXPECT_THROW(obj.ReadRange(22, 0, 10, buf), std::out_of_range);
}