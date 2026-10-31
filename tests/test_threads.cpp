#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "file_collector.hpp"

// Тесты для проверки потокобезопасности класса

// Тест проверяет работу класса в ситуации, когда numThreads потоков отправляют чанки размером numChunkSize
TEST(Threads, OneFile) {
    constexpr size_t numThreads  = 10; // кол-во потоков
    constexpr size_t numChunkSize = 10; // размер чанка
    constexpr size_t numFileSize  = numThreads * numChunkSize; // размер файла

    FileCollector obj;
    obj.CollectFile(1, numFileSize); // регистрируем файл

    std::vector<std::thread> threads; // вектор для хранения потоков
    for (size_t t = 0; t < numThreads; ++t) { 
        threads.emplace_back([&obj, t]() { // создание потока и передача внего лямбды функции
            std::vector<uint8_t> chunk(numChunkSize, static_cast<uint8_t>(t + 1)); //создание чанка, заполненного t+1
            obj.OnNewChunk(1, t * numChunkSize, std::move(chunk));
        });
    }
    for (auto& thr : threads) {
        thr.join(); // ожидание завершения потока
    }
    

    ASSERT_TRUE(obj.IsComplete(1)); // проверяет, собран ли файл
    auto f = obj.GetFile(1); // получение файла
    ASSERT_NE(f, nullptr);
    ASSERT_EQ(f->size(), numFileSize); // проверка размера
    // проверка корректности заполнения файла данными
    for (size_t i = 0; i < numFileSize; ++i) {
        const uint8_t expected = static_cast<uint8_t>(i / numChunkSize + 1);
        ASSERT_EQ((*f)[i], expected) << "at index " << i;
    }
}

// Тест проверяет работу класса в ситуации, когда несколько потоков отправляют чанки размером numChunkSize в разные файлы
TEST(Threads, ManyFiles) {
    constexpr size_t numThreads  = 10; // кол-во потоков
    constexpr size_t kFiles    = 10; // кол-во файлов
    constexpr size_t numFileSize = 21; // размер каждого файла

    static_assert(numThreads == kFiles, "numThreads must equal kFiles"); // кол-во потоков должно быть равно кол-ву файлов

    FileCollector obj; // создание сборщика
    for (size_t i = 0; i < kFiles; ++i) {
        obj.CollectFile(static_cast<uint32_t>(i), numFileSize); // регистрация файлов
    }

    std::vector<std::thread> threads; // вектор потоков
    for (size_t i = 0; i < numThreads; ++i) {
        threads.emplace_back([&obj, i]() { // создание потока и передача в него лямбды функции
            std::vector<uint8_t> chunk(numFileSize, static_cast<uint8_t>(i + 1)); // создание чанка, заполненного i+1
            obj.OnNewChunk(static_cast<uint32_t>(i), 0, std::move(chunk)); // запись чанка
        });
    }
    for (auto& thr : threads) {
        thr.join(); // ожидание заверешения потока
    }

    // проверка каждого файла
    for (size_t i = 0; i < kFiles; ++i) {
        ASSERT_TRUE(obj.IsComplete(static_cast<uint32_t>(i)));
        auto f = obj.GetFile(static_cast<uint32_t>(i));
        ASSERT_NE(f, nullptr);
        ASSERT_EQ(f->size(), numFileSize);

        
        for (size_t j = 0; j < numFileSize; ++j) {
            ASSERT_EQ((*f)[j], static_cast<uint8_t>(i + 1)) // проверяем, что весь файл заполнен значением i+1
                << "file " << i << " at " << j;
        }
    }
}