#ifndef FILE_MANAGER_HPP
#define FILE_MANAGER_HPP

#include <cstdlib>
#include <string>
#include <fstream>
#include <unordered_map>
#include <iostream>
#include <filesystem>

namespace filesystem = std::filesystem;

namespace file {

using Path = filesystem::path;

class FileManager {
public:
    FileManager()
        : _defined_paths {}
    {}

    void create_file(Path path, const std::string filename) {
        std::string filepath = filesystem::relative(path /= filename).string();
        std::cout << filepath << '\n';
        std::ifstream {filepath}.close();
        this->_defined_paths.insert(std::make_pair(path.stem().string(), path));
        return;
    }

    bool create_folder(const std::string pathname) {
        auto path {pathname};
        if (!filesystem::create_directory(path))
            return {false};

        this->_defined_paths.insert(std::make_pair(pathname, path));
        return {true};
    }

    bool delete_file_or_folder(const Path path) {
        return filesystem::remove(path);
    }

    Path get_path(const std::string pathname) {
        if (!filesystem::exists(Path {pathname}))
            return Path {};

        return this->_defined_paths.at(pathname);
    }

    void write_to_file(const std::string write_to, std::ofstream& file_to_write_to) {
        file_to_write_to << write_to;
        return this->close_file(file_to_write_to);
    }

    std::string read_file(std::ifstream& file_to_read_from) {
        return std::string {std::istreambuf_iterator {file_to_read_from}, {}};
    }

    std::fstream open_file(const std::string filepath) {
        return std::fstream {filepath};
    }

    std::ifstream open_file_for_reading(const std::string filepath) {
        return std::ifstream {filepath};
    }

    std::ofstream open_file_for_writing(const std::string filepath) {
        return std::ofstream {filepath};
    }

    void close_file(std::ifstream& file) {
        return file.close();
    }

    void close_file(std::ofstream& file) {
        return file.close();
    }

    void close_file(std::fstream& file) {
        return file.close();
    }

private:
    std::unordered_map<std::string, Path> _defined_paths;
};

} //namespace file

#endif