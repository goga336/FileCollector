#include <cstdint>
#include <gtest/gtest.h>
#include "../inc/file_collector.hpp"

TEST(FileCollector, OneChunk){
    FileCollector obj;
    obj.CollectFile(11, 10);
    EXPECT_FALSE(obj.IsComplete(11));
    EXPECT_EQ(obj.GetFile(11), nullptr);

    std::vector<uint8_t> chunk = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    obj.OnNewChunk(11, 0, std::move(chunk));
    EXPECT_TRUE(obj.IsComplete(11));
    std::shared_ptr<const std::vector<uint8_t>> file = obj.GetFile(11);
    ASSERT_NE(file, nullptr);
    ASSERT_EQ(file->size(), 10u);
    EXPECT_EQ((*file)[0], 9);
    EXPECT_EQ((*file)[1], 8);
    EXPECT_EQ((*file)[2], 7);
    EXPECT_EQ((*file)[3], 6);
    EXPECT_EQ((*file)[4], 5);
    EXPECT_EQ((*file)[6], 3);
    EXPECT_EQ((*file)[7], 2);
    EXPECT_EQ((*file)[8], 1);
    EXPECT_EQ((*file)[9], 0);
}