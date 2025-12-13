#pragma once
#include <boost/filesystem.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <fstream>

class Hash {
private:
    size_t block_size;

    // Структура для хранения открытого файла и его кэша
    struct FileHandle {
        std::unique_ptr<std::ifstream> stream;
        uintmax_t size;
        std::unordered_map<size_t, std::string> block_cache;
        boost::filesystem::path path;

        FileHandle(const boost::filesystem::path& file_path);
        bool is_valid() const { return stream && stream->is_open(); }
    };

    // Хеш-функция CRC32
    std::string hash_crc32(const std::string &data);

    // Получить хеш блока (с автоматическим кэшированием)
    std::string get_block_hash(FileHandle& handle, size_t block_index);

    // Сравнить два файла начиная с определенного блока
    bool compare_handles_from_block(FileHandle& handle1, FileHandle& handle2,
                                   size_t start_block);

public:
    Hash(size_t block_size);

    // Основной метод - находит настоящие дубликаты
    std::vector<std::vector<boost::filesystem::path>>
    find_real_duplicates(std::vector<std::vector<boost::filesystem::path>> size_groups);
};
//Все а дальше уже будет другой класс как раз принимать в конструкторе std::vector<списков>
//и работать с ним бежать по вектору по индексам брать первый список и первый элемент.
//списке полностью его хэшировать блоками и дальше идти к
//следующему в списке элементу и хешировать его блоками и каждый блок проверять с эталоном
//первым который полностью был захэширован)


//окей давай поговорим про объект Hash что он будет уметь делать
//1.Будет основная функция которая будет принимать std::vector<std::vector<boost::filesystem::path>> duplicate_groups а возвращать будет std::vector<std::vector<boost::filesystem::path>> но уже проверенную на содержимое
    //2. ему нужно бежать по вектору
//     for (const auto& duplicate_group : duplicate_groups) {
//    for (const auto& file : duplicate_group) {
//        std::cout << "  - " << file << std::endl;;
//    } и как раз таки тут будет вызываться метод parse_block
//}

//3.
//В этой функции parse_string_block_hash нужно определить сразу словарь который будет содержать ключ это хэш а значение это сама строка из файла размером s) КАК РАЗ ТАКИ ЭТОТ СЛОВАРЬ длины 1
//функция которая будет из файла парсить строку определенной длины S  пусть эта функция будет parse_string_block_hash будет принимать (размер блока и путь к файлу)
//внутри этой функции мы должны открыть файл и считать данные из файла длины s и вызвать hash_block(принимает строку длины s)
//ей понадобиться вспомогательная функция hash_block которая будет возвращать hash блока

// что еще можно сделать функцию эталон parse_Эталон_block_hash хэш  тогда в основной функции при итерировании нудо будет сделать проверку если file это первый в списке duplicate_group то вызывать для него эталонную функцию а если нет то сравнивать с эталоном
//ТАККК ПОНАДОБИТЬСЯ ЕЩЕ ОДНА ВСПОМОГАТЕЛЬНАЯ ФУНКЦИЯ hash_checking В НЕЕ ПЕРЕДАЕТСЯ результат parse_string_block_hash и результат parse_Эталон_block_hash так вот
// эта функция должна вернуть True или False совпадает ли parse_file_block_hash с parse_Эталон_block_hash

// так логика немножка поменялась std::unordered_map<uintmax_t, std::string> parse_string_block_hash(boost::filesystem::path& file_path,size_t block_size); и bool hash_checking(const std::unordered_map<uintmax_t, std::string> map, std::unordered_map<uintmax_t, std::string> reference); объединены


// Ты делаешь:
//1. parse_reference_block_hash() → читает ВЕСЬ эталонный файл сразу
//2. parse_string_block_hash_and_check() → читает ВЕСЬ текущий файл сразу

//Файлы одинакового размера: [F1, F2, F3, F4, F5]

//Алгоритм:
//1. Берем F1 - ничего не делаем (нет с чем сравнивать)
//2. Берем F2: сравниваем F1 и F2 блок за блоком
//   - Если одинаковые → они в одной группе [F1, F2]
//   - Если разные → F2 начинает новую группу [F2]
//3. Берем F3: сравниваем с каждой существующей группой:
//   - Сначала с группой [F1, F2]: сравниваем с F1 (представителем группы)
//     * Если совпал → добавляем в группу [F1, F2, F3]
//     * Если не совпал → пробуем следующую группу
//   - Или с группой [F2] и т.д.
//4. Повторяем для всех файлов



//Я реализовал алгоритм, который сначала группирует файлы по размеру,
//а затем внутри каждой размерной группы выполняет полное попарное сравнение по содержимому.
//Файлы с одинаковым содержимым объединяются в группы.
//Сравнение происходит блок за блоком с кэшированием хешей,
// что гарантирует, что каждый блок каждого файла читается с диска не более одного раза.