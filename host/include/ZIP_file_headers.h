#pragma once
#pragma pack(push, 1)

#include <cstdint>
#include <memory>
#include <vector>
#include <iostream>
#include <cassert>

enum class valid_signatures : uint32_t {
    LFH = 0x04034b50,
	DD = 0x08074b50,
	CDFH = 0x02014b50,
	EOCD = 0x06054b50,
    EOCD64 = 0x06064b50,
    Locator = 0x07064b50
};

struct extraFieldRecord {
    uint16_t signature;
    uint16_t size;
    std::vector<uint8_t> data;
};

struct LocalFileHeader {
    // Минимальная версия для распаковки
    uint16_t versionToExtract;
    // Битовый флаг
    uint16_t generalPurposeBitFlag;
    // Метод сжатия (0 - без сжатия, 8 - deflate)
    uint16_t compressionMethod;
    // Время модификации файла
    uint16_t modificationTime;
    // Дата модификации файла
    uint16_t modificationDate;
    // Контрольная сумма
    uint32_t crc32;
    // Сжатый размер
    uint32_t compressedSize;
    // Несжатый размер
    uint32_t uncompressedSize;
    // Длина название файла
    uint16_t filenameLength;
    // Длина поля с дополнительными данными
    uint16_t extraFieldLength;
    // Название файла (размером filenameLength)
    std::vector<uint8_t> filename;
    // Дополнительные данные (размером extraFieldLength)
    std::vector<extraFieldRecord> extraField;

    bool is_64() const;
};

struct File {
    std::vector<uint8_t> data;
    uint16_t compressionMethod;
    uint64_t compressedSize;
    uint64_t uncompressedSize;
};

struct DataDescriptor {
    // Контрольная сумма
    uint32_t crc32;
    // Сжатый размер
    uint32_t compressedSize;
    // Несжатый размер
    uint32_t uncompressedSize;
};

struct CentralDirectoryFileHeader {
    // Версия для создания
    uint16_t versionMadeBy;
    // Минимальная версия для распаковки
    uint16_t versionToExtract;
    // Битовый флаг
    uint16_t generalPurposeBitFlag;
    // Метод сжатия (0 - без сжатия, 8 - deflate)
    uint16_t compressionMethod;
    // Время модификации файла
    uint16_t modificationTime;
    // Дата модификации файла
    uint16_t modificationDate;
    // Контрольная сумма
    uint32_t crc32;
    // Сжатый размер
    uint32_t compressedSize;
    // Несжатый размер
    uint32_t uncompressedSize;
    // Длина название файла
    uint16_t filenameLength;
    // Длина поля с дополнительными данными
    uint16_t extraFieldLength;
    // Длина комментариев к файлу
    uint16_t fileCommentLength;
    // Номер диска
    uint16_t diskNumber;
    // Внутренние аттрибуты файла
    uint16_t internalFileAttributes;
    // Внешние аттрибуты файла
    uint32_t externalFileAttributes;
    // Смещение до структуры LocalFileHeader
    uint32_t localFileHeaderOffset;
    // Имя файла (длиной filenameLength)
    std::vector<uint8_t> filename;
    // Дополнительные данные (длиной extraFieldLength)
    std::vector<extraFieldRecord> extraField;
    // Комментарий к файла (длиной fileCommentLength)
    std::vector<uint8_t> fileComment;

    bool is_64() const;

    extraFieldRecord& get_efr_of_ei();
};

struct ZIP64EI{
    ZIP64EI(const std::vector<uint8_t> &data, const CentralDirectoryFileHeader &cdfh);
    ZIP64EI(const std::vector<uint8_t> &data, const LocalFileHeader &cdfh);
    ZIP64EI() = default;

    int to_vector(std::vector<uint8_t> &res, CentralDirectoryFileHeader &cdfh);
    int to_vector(std::vector<uint8_t> &res, LocalFileHeader &cdfh);
    // Размер несжатых данных
    uint64_t uncompressedSize;
    // Размер сжатых данных
    uint64_t compressedSize;
    // Смещение Local File Header от начала файла
    uint64_t localFileHeaderOffset;
    // Номер диска для поиска
    uint32_t diskNumber;
};

struct EOCD {
    // Номер диска
    uint16_t diskNumber;
    // Номер диска, где находится начало Central Directory
    uint16_t startDiskNumber;
    // Количество записей в Central Directory в текущем диске
    uint16_t numberCentralDirectoryRecord;
    // Всего записей в Central Directory
    uint16_t totalCentralDirectoryRecord;
    // Размер Central Directory
    uint32_t sizeOfCentralDirectory;
    // Смещение Central Directory
    uint32_t centralDirectoryOffset;
    // Длина комментария
    uint16_t commentLength;
    // Комментарий (длиной commentLength)
    std::vector<uint8_t> comment;

    bool is_64() const;
};

struct EOCD64Locator {
    // Номер диска для поиска EOCD64
    uint32_t diskNumber;
    // Смещение от начала файла до EOCD64
    uint64_t eocd64Offset;
    // Количество дисков
    uint32_t totalDiskCount;
};

struct EOCD64 {
    EOCD64() = default;
    EOCD64(const EOCD &eocd);
    // Размер записи EOCD64
    uint64_t eocd64Size;
    // Версия для создания
    uint16_t versionMadeBy;
    // Версия для распаковки
    uint16_t versionToExtract;
    // Номер текущего диска
    uint32_t diskNumber;
    // Номер диска для поиска Central Directory
    uint32_t startDiskNumber;
    // Количество записей в Central Directory
    uint64_t numberCentralDirectoryRecord;
    // Всего записей в Central Directory
    uint64_t totalCentralDirectoryRecord;
    // Размер Central Directory
    uint64_t sizeOfCentralDirectory;
    // Смещение Central Directory
    uint64_t centralDirectoryOffset;
    // zip64 extensible data sector (переменной длины)
    std::vector<uint8_t> data_sector;
};
#pragma pack(pop)