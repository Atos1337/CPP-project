#pragma once
#pragma pack(push, 1)

#include <cstdint>
#include <memory>
#include <vector>

enum class valid_signatures : uint32_t {
    LFH = 0x04034b50,
	DD = 0x08074b50,
	CDFH = 0x02014b50,
	EOCD = 0x06054b50,
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
};

struct File {
    std::vector<uint8_t> data;
    uint16_t compressionMethod;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
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
};
#pragma pack(pop)