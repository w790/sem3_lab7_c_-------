#include <iostream>
#include <boost/program_options.hpp>
#include <vector>
#include <string>
#include "ScannerDirectory.h"
#include "Hash.h"

//$ bayan [...]
//--include или -i - директории для сканирования
//--exclude или -e - директории для исключения
//--level или -l - уровень сканирования
//--min-size или -m - минимальный размер файла
//--mask - маски файлов
//--block-size или -S - размер блока (S)
//--hash или -H алгоритм хеширования (crc32/md5)//буду юзать только crc32

namespace po = boost::program_options;

int main(int argc, char* argv[]){
    // Для скалярных значений - используйте переменные
    // Для списков - извлекайте через vm
    int level = 0;
    size_t min_size = 1;
    size_t block_size = 1024;
    std::string hash_algo = "crc32";

    std::vector<std::string> include_dirs;
    std::vector<std::string> exclude_dirs;
    std::vector<std::string> masks;

    po::options_description desc("General options");
    desc.add_options()
        ("include,i", po::value<std::vector<std::string>>(&include_dirs)->multitoken(), "директории для сканирования")//po::value<T>() - создает хранилище для значения типа T
        ("exclude,e", po::value<std::vector<std::string>>(&exclude_dirs)->multitoken(), "директории для исключения")//нет указателя на переменную значение нужно извлекать через vm["exclude"]
        ("level,l", po::value<int>(&level)->default_value(0), "уровень сканирования")//default_value(x) - значение по умолчанию
        ("min-size,m", po::value<size_t>(&min_size)->default_value(1), "минимальный размер файла")
        ("mask", po::value<std::vector<std::string>>(&masks)->multitoken(), "маски файлов")//multitoken() - параметр может принимать несколько значений
        ("block-size,S", po::value<size_t>(&block_size)->default_value(1024), "размер блока (S)")
        ("hash,H", po::value<std::string>(&hash_algo)->default_value("crc32"), "алгоритм хеширования crc32");

    po::variables_map storage;//контейнер для хранения распарсенных значений

    // Парсим
    po::store(po::parse_command_line(argc, argv, desc), storage);
    po::notify(storage);

    // Отладочный вывод
    std::cout << "Параметры сканирования" << std::endl;
    std::cout << "Директории для сканирования: ";
    for (const auto& dir : include_dirs){
         std::cout << dir << " ";
    }

    std::cout << std::endl;

    std::cout << "Исключенные директории: ";
    for (const auto& dir : exclude_dirs){
        std::cout << dir << " ";
    }
    std::cout << std::endl;

    std::cout << "Маски: ";
    for (const auto& mask : masks){
        std::cout << mask << " ";
    }
    std::cout << std::endl;

    std::cout << "Уровень: " << level << std::endl;
    std::cout << "Мин. размер: " << min_size << " байт" << std::endl;
    std::cout << "Размер блока: " << block_size << " байт" << std::endl;
    std::cout << "Алгоритм хеширования: " << hash_algo << std::endl;

    //создаем сканер
    ScannerDirectory scanner(level, min_size, masks, exclude_dirs);

    // Сканируем
    auto files = scanner.scan_directories(include_dirs);

    // Выводим результат
    std::cout << "\nРезультаты сканирования " << std::endl;
    std::cout << "Найдено файлов: " << files.size() << std::endl;
    for (const auto& file : files) {
        std::cout << "  - " << file.string() << std::endl;
    }

    //теперь выведем с сортировкой по размеру
    std::vector<std::vector<boost::filesystem::path>> duplicate_groups = scanner.get_duplicate_groups_by_size(files);

    std::cout << "\nРезультаты сканирования с группировкой по размеру" << std::endl;
    std::cout << "Найдено файлов: " << files.size() << std::endl;
    for (const auto& duplicate_group : duplicate_groups) {
        for (const auto& file : duplicate_group) {
            std::cout << "  - " << file << std::endl;;
        }
    }


    // Проверяем хеши файлов
    std::cout << "\n=== Проверка хешей файлов ===" << std::endl;

    // Создаем объект Hash с указанным размером блока
    Hash hash_checker(block_size);

    // Проверяем группы файлов на настоящие дубликаты
    auto final_duplicate_groups = hash_checker.hash_directories(duplicate_groups);

    std::cout << "\n=== Окончательные результаты (с проверкой хешей) ===" << std::endl;
    std::cout << "Найдено групп настоящих дубликатов: " << final_duplicate_groups.size() << std::endl;

    if (final_duplicate_groups.empty()) {
        std::cout << "Дубликаты не найдены" << std::endl;
    } else {
        int group_number = 1;
        for (const auto& group : final_duplicate_groups) {
            std::cout << "\nГруппа " << group_number++ << " (" << group.size() << " файлов):" << std::endl;
            for (size_t i = 0; i < group.size(); ++i) {
                std::cout << "  " << (i + 1) << ". " << group[i].string()
                          << " (" << boost::filesystem::file_size(group[i]) << " байт)" << std::endl;
            }
        }
    }

    return 0;
}