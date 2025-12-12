#pragma once
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include <unordered_map>

class ScannerDirectory{
private:
    int max_level_scan;
    size_t min_file_size_;
    std::vector<std::string> masks_;
    std::vector<std::string> exclude_dirs_;

    bool is_excluded(const boost::filesystem::path& path);//проверка исключена ли директория
    bool matches_mask(const std::string& filename);// проверяет маску

public:
    ScannerDirectory(int max_level_scan,
                    size_t min_file_size,
                    std::vector<std::string> masks,
                    std::vector<std::string> exclude_dirs);

    std::vector<boost::filesystem::path> scan_single_directory(const boost::filesystem::path& dir_path);
    std::vector<boost::filesystem::path> scan_directories(const std::vector<std::string>& dirs_to_scan);

    std::unordered_map<uintmax_t, std::vector<boost::filesystem::path>> group_files_by_size(const std::vector<boost::filesystem::path>& files);//Функция для группировки файлов по размеру
    std::vector<std::vector<boost::filesystem::path>> get_duplicate_groups_by_size(const std::vector<boost::filesystem::path>& files);//Функция для преобразования в вектор списков (только дубликаты по размеру)
};