#include "../inc/file_collector.hpp"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

static int failures = 0;

#define CHECK(cond) do {                                                    \
    if (!(cond)) {                                                          \
        std::cerr << "FAIL line " << __LINE__ << ": " #cond "\n";           \
        ++failures;                                                         \
    }                                                                       \
} while (0)

#define CHECK_THROW(expr, extype) do {                                      \
    try { expr;                                                             \
        std::cerr << "FAIL line " << __LINE__                               \
                  << ": no throw, expected " #extype "\n";                  \
        ++failures;                                                         \
    } catch (const extype&) { /* ok */ }                                    \
      catch (...) {                                                         \
        std::cerr << "FAIL line " << __LINE__                               \
                  << ": wrong exception type, expected " #extype "\n";      \
        ++failures;                                                         \
    }                                                                       \
} while (0)

int main() {
    // ---------- 1. По порядку: два чанка ----------
    {
        FileCollector fc;
        fc.CollectFile(1, 10);
        CHECK(!fc.IsComplete(1));
        CHECK(fc.GetFile(1) == nullptr);

        std::vector<uint8_t> c1 = {1,2,3,4};
        fc.OnNewChunk(1, 0, std::move(c1));
        CHECK(!fc.IsComplete(1));
        CHECK(fc.GetFile(1) == nullptr);

        std::vector<uint8_t> c2 = {5,6,7,8,9,10};
        fc.OnNewChunk(1, 4, std::move(c2));
        CHECK(fc.IsComplete(1));

        auto f = fc.GetFile(1);
        CHECK(f != nullptr);
        CHECK(f->size() == 10);
        if (f) for (int i = 0; i < 10; ++i) CHECK((*f)[i] == (uint8_t)(i + 1));
    }

    // ---------- 2. Пример из задания: пересечения ----------
    {
        FileCollector fc;
        fc.CollectFile(7, 10);

        std::vector<uint8_t> a = {'A','B','C','D'};
        fc.OnNewChunk(7, 0, std::move(a));
        CHECK(!fc.IsComplete(7));

        std::vector<uint8_t> b = {'C','D','E','F'};
        fc.OnNewChunk(7, 2, std::move(b));
        CHECK(!fc.IsComplete(7));

        std::vector<uint8_t> c = {'G','H','I','J'};
        fc.OnNewChunk(7, 6, std::move(c));
        CHECK(fc.IsComplete(7));

        auto f = fc.GetFile(7);
        CHECK(f != nullptr);
        const char* expected = "ABCDEFGHIJ";
        if (f) for (int i = 0; i < 10; ++i) CHECK((*f)[i] == (uint8_t)expected[i]);
    }

    // ---------- 3. Дубликаты (тот же чанк дважды) ----------
    {
        FileCollector fc;
        fc.CollectFile(3, 4);
        std::vector<uint8_t> a = {1,2,3,4};
        fc.OnNewChunk(3, 0, std::move(a));
        CHECK(fc.IsComplete(3));

        // Повторный тот же чанк — не должно ломать count
        std::vector<uint8_t> a2 = {1,2,3,4};
        fc.OnNewChunk(3, 0, std::move(a2));
        CHECK(fc.IsComplete(3));

        auto f = fc.GetFile(3);
        CHECK(f != nullptr);
        if (f) for (int i = 0; i < 4; ++i) CHECK((*f)[i] == (uint8_t)(i + 1));
    }

    // ---------- 4. Пустой файл ----------
    {
        FileCollector fc;
        fc.CollectFile(2, 0);
        CHECK(fc.IsComplete(2));
        auto f = fc.GetFile(2);
        CHECK(f != nullptr);
        if (f) CHECK(f->empty());
    }

    // ---------- 5. Пустой чанк — no-op ----------
    {
        FileCollector fc;
        fc.CollectFile(1, 5);
        std::vector<uint8_t> empty;
        fc.OnNewChunk(1, 0, std::move(empty));
        CHECK(!fc.IsComplete(1));
        CHECK(fc.GetFile(1) == nullptr);
    }

    // ---------- 6. Ошибки ----------
    {
        FileCollector fc;
        fc.CollectFile(1, 10);

        // Повторная регистрация
        CHECK_THROW(fc.CollectFile(1, 10), std::invalid_argument);

        // Неизвестный fileId
        CHECK_THROW(fc.OnNewChunk(99, 0, std::vector<uint8_t>{1}), std::out_of_range);

        // За границами: pos+size > fileSize
        std::vector<uint8_t> big(5, 0);
        CHECK_THROW(fc.OnNewChunk(1, 8, std::move(big)), std::out_of_range);

        // pos > fileSize
        std::vector<uint8_t> one = {1};
        CHECK_THROW(fc.OnNewChunk(1, 100, std::move(one)), std::out_of_range);

        // RemoveFile неизвестного
        CHECK_THROW(fc.RemoveFile(99), std::out_of_range);
    }

    // ---------- 7. RemoveFile ----------
    {
        FileCollector fc;
        fc.CollectFile(1, 4);
        std::vector<uint8_t> a = {1,2,3,4};
        fc.OnNewChunk(1, 0, std::move(a));
        fc.RemoveFile(1);
        CHECK_THROW(fc.IsComplete(1), std::out_of_range);
        CHECK_THROW(fc.GetFile(1), std::out_of_range);
        CHECK_THROW(fc.OnNewChunk(1, 0, std::vector<uint8_t>{1}), std::out_of_range);
    }

    // ---------- 8. Пересечение с перекрытием + не по порядку ----------
    {
        FileCollector fc;
        fc.CollectFile(5, 8);

        // сначала конец
        std::vector<uint8_t> tail = {5,6,7,8};
        fc.OnNewChunk(5, 4, std::move(tail));
        CHECK(!fc.IsComplete(5));

        // потом начало с перекрытием на байт 4
        std::vector<uint8_t> head = {1,2,3,4,5};
        fc.OnNewChunk(5, 0, std::move(head));
        CHECK(fc.IsComplete(5));

        auto f = fc.GetFile(5);
        CHECK(f != nullptr);
        if (f) for (int i = 0; i < 8; ++i) CHECK((*f)[i] == (uint8_t)(i + 1));
    }

    if (failures == 0) {
        std::cout << "ALL TESTS PASSED\n";
        return 0;
    }
    std::cerr << failures << " FAILURES\n";
    return 1;
}