#pragma once
#include <boost/filesystem.hpp>
#include <vector>
#include <string>
#include <unordered_map>

class Hash {
private:
    size_t block_size;
    std::string hash_block(const std::string &str_lenght_s);

    std::unordered_map<uintmax_t, std::string> parse_reference_block_hash(boost::filesystem::path& file_path);

    bool parse_string_block_hash_and_check(boost::filesystem::path& file_path, const std::unordered_map<uintmax_t, std::string>& reference_hashes);


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
public:
    Hash(size_t block_size);
    std::vector<std::vector<boost::filesystem::path>> hash_directories(std::vector<std::vector<boost::filesystem::path>>);
};
