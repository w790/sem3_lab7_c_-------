#include "Hash.h"
#include <boost/crc.hpp>
#include <sstream>
#include <fstream>
#include <iomanip>

Hash::Hash(size_t block_size) : block_size(block_size) {}

std::string Hash::hash_block(const std::string &str_lenght_s){
    // TODO: Вычислить CRC32 для строки str_lenght_s
    // Использовать boost::crc_32_type
    // Результат преобразовать в строку в hex формате

    //создание объекта boost::crc_32_type
    boost::crc_32_type result_crc;
    //обработка байтов строки str_lenght_s
    result_crc.process_bytes(str_lenght_s.data(),str_lenght_s.length());
    //получить checksum()
    uint32_t checksum = result_crc.checksum();
    //преобразование в hex строку
    std::stringstream ss;
    ss << std::hex << std::setw(8) << std::setfill('0') << checksum;

    return ss.str();
}

std::unordered_map<uintmax_t, std::string> Hash::parse_reference_block_hash(boost::filesystem::path& file_path){
    std::unordered_map<uintmax_t, std::string> result;
    // нам надо открыть файл в бинарном режими и начать по нему проходить
    std::ifstream ifs(file_path.string(), std::ios::binary);

    if (!ifs.is_open()) {
        return result;
    }


    uintmax_t block_index = 0;

    while (!ifs.eof()) {
        std::vector<char> buffer(block_size);
        ifs.read(buffer.data(), block_size);
        std::streamsize bytes_read = ifs.gcount();

        if (bytes_read == 0){
            break;
        }

        std::string block_data(buffer.begin(), buffer.begin() + bytes_read);

        result[block_index] = hash_block(block_data);
        block_index++;
    }

    ifs.close();
    return result;
}

bool Hash::parse_string_block_hash_and_check(boost::filesystem::path& file_path, const std::unordered_map<uintmax_t, std::string>& reference_hashes){
    std::ifstream ifs(file_path.string(), std::ios::binary);
\
    if (!ifs.is_open()) {
        return false;
    }

    uintmax_t block_index = 0;

    while (!ifs.eof()) {
        std::vector<char> buffer(block_size);
        ifs.read(buffer.data(), block_size);
        std::streamsize bytes_read = ifs.gcount();
        if (bytes_read == 0) {
            break;
        }
        std::string block_data(buffer.begin(), buffer.begin() + bytes_read);
        std::string current_hash = hash_block(block_data);

        auto it = reference_hashes.find(block_index);
        if (it == reference_hashes.end()) {
            ifs.close();
            return false;
        }
        if (it->second != current_hash) {
            ifs.close();
            return false;
        }
        block_index++;
    }
    ifs.close();
    if (block_index != reference_hashes.size()) {
        return false;
    }
    return true;
}


std::vector<std::vector<boost::filesystem::path>> Hash::hash_directories(std::vector<std::vector<boost::filesystem::path>> duplicate_groups){
    std::vector<std::vector<boost::filesystem::path>> RESULTTTTTTTT;
    for (auto& group : duplicate_groups) {
        if (group.size() < 2) {
            continue;
        }

        boost::filesystem::path reference_file = group[0];
        auto reference_hashes = parse_reference_block_hash(reference_file);

        //группа для настоящих дубликатов
        std::vector<boost::filesystem::path> duplicate_group;
        duplicate_group.push_back(reference_file);  //эталон с нулевой добавляется

        for (size_t i = 1; i < group.size(); i++) {
            if (parse_string_block_hash_and_check(group[i], reference_hashes)) {
                duplicate_group.push_back(group[i]);    //настоящий дубликат
            }
        }

        if (duplicate_group.size() > 1) {
            RESULTTTTTTTT.push_back(duplicate_group);
        }
    }

    return RESULTTTTTTTT;
}