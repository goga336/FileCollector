#ifndef MYPROJECT_FILE_COLLECTOR_HPP
#define MYPROJECT_FILE_COLLECTOR_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

/**
 * @file file_collector.hpp
 * @brief Объявление класса FileCollector
 */

 /**
 * @class FileCollector
 * @brief Потокобезопасный класс, который собирает файлы, передаваемые по сети
 * чанками в произвольном порядке. Один файл может собираться несколькими потоками 
 * одновременно. Класс является потокобезопасным и обеспечивает эффективное восстановление
 * файла из полученных чанков. Потокобезопасность класса реализуется через два мьютекса:
 * - @c filesMx — мьютекс для защиты хэш-таблицы файлов от одновременного
 *   поиска, вставки и удаления из разных потоков;
 * - @c mx — мьютекс для защиты содержимого конкретного файла от гонок при параллельной
 * записи чанков.
 */

class FileCollector{
    private:
    /**
     * @brief Запись в хэш-таблице файлов
     */
    struct File{
        mutable std::mutex mx; ///< Мьютекс для защиты содержимого файла
        std::vector<uint8_t> data; ///< Буфер файла
        std::vector<bool> bitMask; ///< Маска приема чанков, используется для учета дубликатов/пересечений
        size_t count = 0; ///< Число уникальных полученных байт, используется для проверки целостности файла
    };
    mutable std::mutex filesMx; ///< Мьютекс для защиты хэш-таблицы файлов
    std::unordered_map<uint32_t, std::shared_ptr<File>> files; ///< Хэш-таблица для хранения/поиска зарегистрированных файлов
    
    /**
     * @brief Метод поиска файла по @p fileId в хэш-таблице
     * Поиск выполняется с захваченным мьютексом
     * @param fileId Идентификатор файла
     * @return @c shared_ptr на запись в хэш-таблице файлов
     * @throws std::out_of_range если @p fileId не зарегистрирован
     */

    std::shared_ptr<File> FindFile(uint32_t fileId) const;
    public:
    FileCollector() = default;
    ~FileCollector() = default;
    FileCollector(const FileCollector&) = delete;
    FileCollector& operator=(const FileCollector&) = delete;

    /**
     * @brief Метод создания структуры для сбора чанков
     * Метод выделяет буфер под размер файла, создает маску
     * @param fileId Идентификатор файла
     * @param fileSize Размер файла
     * @throws std::invalid_argument если файл с таким fileId уже зарегистрирован
     * @throws std::runtime_error    если не хватает памяти под буфер
     */

    void CollectFile(uint32_t fileId, size_t fileSize);

    /**
     * @brief Метод добавления чанка
     * @param fileId Идентификатор файла
     * @param pos    Позиция чанка в файле
     * @param chunk  Данные чанка
     * @throws std::out_of_range если файл с таким fileId неизвестен или чанк выходит за границы
     */

    void OnNewChunk(uint32_t fileId, size_t pos, std::vector<uint8_t> &&chunk);

    /**
     * @brief Метод проверки готовности файла
     * @param fileId Идентификатор файла
     * @return true если все байты файла получены
     * @throws std::out_of_range если файл с таким fileId неизвестен
     */

    bool IsComplete(uint32_t fileId) const;

    /**
     * @brief Метод получения собранного файла
     * Данный метод возвращает shared_ptr, который указывает на буфер, 
     * который хранится внутри хэш-таблицы, благодаря чему избегается полное 
     * копирование буфера
     * @param fileId Идентификатор файла
     * @return shared_ptr возвращаем умный указатель на буфер файла, если
     * файл собран, в противном случае возвращаем - nullptr
     * @throws std::out_of_range если файл с таким fileId неизвестен
     */

    std::shared_ptr<const std::vector<uint8_t>> GetFile(uint32_t fileId) const;

    /**
     * @brief Метод удаления файла
     * @param fileId Идентификатор файла
     * @throws std::out_of_range если файл с таким fileId неизвестен
     */

    void RemoveFile(uint32_t fileId);

    /**
     * @brief Метод частичного получения файла
     * Данный метод позволяет получить неполный файл в рамках указанного промежутка
     * @param fileId Идентификатор файла
     * @param pos    Начальная позиция диапазона в файле
     * @param len  Длина диапазона
     * @param outBuf  Принимающий буфер
     * @return true, если весь диапазон уже получен и скопирован, иначе false
     * @throws std::invalid_argument если outBuf это nullptr
     * @throws std::out_of_range если файл с таким fileId неизвестен или диапазон
     * выходит за границы файла
     */

    bool ReadRange(uint32_t fileId, size_t pos, size_t len, uint8_t* outBuf) const;
    

};
#endif 