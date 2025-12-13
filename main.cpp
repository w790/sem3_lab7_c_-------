#include <iostream>
#include <boost/program_options.hpp>
#include <vector>
#include <string>
#include "ScannerDirectory.h"
#include "Hash.h"

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    // Параметры
    int level = 0;
    size_t min_size = 1;
    size_t block_size = 1024;
    std::string hash_algo = "crc32";

    std::vector<std::string> include_dirs;
    std::vector<std::string> exclude_dirs;
    std::vector<std::string> masks;

    // Настройка парсера командной строки
    po::options_description desc("Опции программы");
    desc.add_options()
        ("include,i", po::value<std::vector<std::string>>(&include_dirs)->multitoken(),
         "директории для сканирования (можно несколько)")
        ("exclude,e", po::value<std::vector<std::string>>(&exclude_dirs)->multitoken(),
         "директории для исключения (можно несколько)")
        ("level,l", po::value<int>(&level)->default_value(0),
         "уровень сканирования (0 - только указанная директория)")
        ("min-size,m", po::value<size_t>(&min_size)->default_value(1),
         "минимальный размер файла в байтах")
        ("mask", po::value<std::vector<std::string>>(&masks)->multitoken(),
         "маски файлов (например, *.txt)")
        ("block-size,S", po::value<size_t>(&block_size)->default_value(1024),
         "размер блока для чтения файлов")
        ("hash,H", po::value<std::string>(&hash_algo)->default_value("crc32"),
         "алгоритм хеширования (crc32)")
        ("help,h", "показать справку");

    po::variables_map vm;

    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return 0;
        }

        // Проверка обязательных параметров
        if (include_dirs.empty()) {
            std::cerr << "Ошибка: не указаны директории для сканирования" << std::endl;
            std::cout << desc << std::endl;
            return 1;
        }

    } catch (const po::error& e) {
        std::cerr << "Ошибка парсинга параметров: " << e.what() << std::endl;
        std::cout << desc << std::endl;
        return 1;
    }

    // Вывод параметров (для отладки, можно убрать)
    std::cout << "=== Параметры сканирования ===" << std::endl;
    std::cout << "Директории для сканирования: ";
    for (const auto& dir : include_dirs) std::cout << dir << " ";
    std::cout << std::endl;

    if (!exclude_dirs.empty()) {
        std::cout << "Исключенные директории: ";
        for (const auto& dir : exclude_dirs) std::cout << dir << " ";
        std::cout << std::endl;
    }

    if (!masks.empty()) {
        std::cout << "Маски файлов: ";
        for (const auto& mask : masks) std::cout << mask << " ";
        std::cout << std::endl;
    }

    std::cout << "Уровень сканирования: " << level << std::endl;
    std::cout << "Мин. размер файла: " << min_size << " байт" << std::endl;
    std::cout << "Размер блока: " << block_size << " байт" << std::endl;
    std::cout << "Алгоритм хеширования: " << hash_algo << std::endl;
    std::cout << "=============================" << std::endl;

    // Создаем сканер и сканируем директории
    ScannerDirectory scanner(level, min_size, masks, exclude_dirs);
    auto files = scanner.scan_directories(include_dirs);

    if (files.empty()) {
        std::cout << "Файлы не найдены" << std::endl;
        return 0;
    }

    // Группируем файлы по размеру
    auto duplicate_groups = scanner.get_duplicate_groups_by_size(files);

    if (duplicate_groups.empty()) {
        std::cout << "Файлы одинакового размера не найдены" << std::endl;
        return 0;
    }

    // Проверяем хеши файлов на совпадение
    Hash hash_checker(block_size);
    auto final_duplicate_groups = hash_checker.find_real_duplicates(duplicate_groups);

    // Выводим результат в требуемом формате
    if (final_duplicate_groups.empty()) {
        std::cout << "Дубликаты не найдены" << std::endl;
        return 0;
    }

    std::cout << "\n=== Найдены дубликаты ===" << std::endl;

    // Формат вывода по заданию: один файл на строку, группы разделены пустой строкой
    for (size_t i = 0; i < final_duplicate_groups.size(); ++i) {
        const auto& group = final_duplicate_groups[i];

        for (const auto& file_path : group) {
            std::cout << file_path.string() << std::endl;
        }

        // Пустая строка между группами (но не после последней)
        if (i != final_duplicate_groups.size() - 1) {
            std::cout << std::endl;
        }
    }

    return 0;
}