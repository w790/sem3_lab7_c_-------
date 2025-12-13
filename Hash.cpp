#include "Hash.h"
#include <boost/crc.hpp>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <algorithm>

Hash::Hash(size_t block_size) : block_size(block_size) {}

// Конструктор FileHandle
Hash::FileHandle::FileHandle(const boost::filesystem::path& file_path)
    : path(file_path), size(0) {
    try {
        size = boost::filesystem::file_size(file_path);
        stream = std::make_unique<std::ifstream>(file_path.string(), std::ios::binary);
        if (!stream->is_open()) {
            stream.reset();
        }
    } catch (const std::exception& e) {
        // Молчим об ошибках, они обрабатываются в is_valid()
        stream.reset();
    }
}

std::string Hash::hash_crc32(const std::string &data) {
    boost::crc_32_type crc_calculator;
    crc_calculator.process_bytes(data.data(), data.size());
    uint32_t checksum = crc_calculator.checksum();

    std::stringstream ss;
    ss << std::hex << std::setw(8) << std::setfill('0') << checksum;
    return ss.str();
}

std::string Hash::get_block_hash(FileHandle& handle, size_t block_index) {
    // Проверяем кэш
    auto it = handle.block_cache.find(block_index);
    if (it != handle.block_cache.end()) {
        return it->second;
    }

    // Проверяем, что файл открыт
    if (!handle.is_valid()) {
        return "";
    }

    // Вычисляем позицию и размер для чтения
    std::ifstream& file = *handle.stream;
    file.clear();
    file.seekg(block_index * block_size, std::ios::beg);

    // Определяем, сколько байт нужно прочитать
    uintmax_t bytes_to_read = block_size;
    uintmax_t file_pos = block_index * block_size;
    if (file_pos + block_size > handle.size) {
        bytes_to_read = handle.size - file_pos;
    }

    if (bytes_to_read == 0) {
        return "";
    }

    // Читаем данные
    std::vector<char> buffer(bytes_to_read);
    file.read(buffer.data(), bytes_to_read);
    std::streamsize bytes_read = file.gcount();

    if (bytes_read == 0) {
        return "";
    }

    // Создаём полный блок размера S с нулями
    std::vector<char> full_block(block_size, '\0');
    std::copy(buffer.begin(), buffer.begin() + bytes_read, full_block.begin());

    // Вычисляем хеш от полного блока
    std::string hash = hash_crc32(std::string(full_block.data(), block_size));

    // Кэшируем результат
    handle.block_cache[block_index] = hash;
    return hash;
}

bool Hash::compare_handles_from_block(FileHandle& handle1, FileHandle& handle2,
                                     size_t start_block) {
    // Проверяем размеры
    if (handle1.size != handle2.size) {
        return false;
    }

    // Вычисляем количество блоков
    size_t total_blocks = (handle1.size + block_size - 1) / block_size;

    // Сравниваем блок за блоком, начиная с start_block
    for (size_t block_idx = start_block; block_idx < total_blocks; block_idx++) {
        std::string hash1 = get_block_hash(handle1, block_idx);
        std::string hash2 = get_block_hash(handle2, block_idx);

        if (hash1.empty() || hash2.empty()) {
            // Один из файлов закончился раньше
            return false;
        }

        if (hash1 != hash2) {
            // Блоки разные - файлы разные
            return false;
        }
    }

    return true;
}

std::vector<std::vector<boost::filesystem::path>>
Hash::find_real_duplicates(std::vector<std::vector<boost::filesystem::path>> size_groups) {
    std::vector<std::vector<boost::filesystem::path>> result;

    for (const auto& group : size_groups) {
        if (group.size() < 2) continue;

        // Создаем хэндлы для всех файлов
        std::vector<FileHandle> handles;
        handles.reserve(group.size());

        for (const auto& path : group) {
            handles.emplace_back(path);
        }

        // Вектор для отслеживания уже обработанных файлов
        std::vector<bool> processed(handles.size(), false);

        // Проходим по всем файлам и ищем дубликаты для каждого
        for (size_t i = 0; i < handles.size(); ++i) {
            if (processed[i] || !handles[i].is_valid()) {
                continue;
            }

            // Группа дубликатов для текущего файла
            std::vector<boost::filesystem::path> duplicate_group;
            duplicate_group.push_back(group[i]);

            // Ищем дубликаты среди оставшихся файлов
            for (size_t j = i + 1; j < handles.size(); ++j) {
                if (processed[j] || !handles[j].is_valid()) {
                    continue;
                }

                // Сравниваем два файла
                if (compare_handles_from_block(handles[i], handles[j], 0)) {
                    duplicate_group.push_back(group[j]);
                    processed[j] = true;  // Помечаем как обработанный
                }
            }

            // Если нашли дубликаты (больше одного файла в группе)
            if (duplicate_group.size() > 1) {
                result.push_back(std::move(duplicate_group));
            }

            processed[i] = true;
        }
    }

    return result;
}