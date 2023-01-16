#include <fstream>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <filesystem>

#include "file manager/file_manager.hpp"

int main() {
  file::FileManager fileManager {};

  const std::string filename = "holaMundo.txt";
  fileManager.create_file(std::filesystem::current_path(), filename);
  std::ofstream hola_mundo_txt = fileManager.open_file_for_writing("./holaMundo.txt");
  fileManager.write_to_file("Hola mundo!", hola_mundo_txt);
  fileManager.close_file(hola_mundo_txt);

  return 0;
}