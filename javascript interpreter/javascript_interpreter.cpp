#include <fstream>
#include "javascript_interpreter.h"

//Constructor Definition
Javascript_interpreter::Javascript_interpreter(std::string files_path) {
  this->files_path = files_path;
};

//Methods Definition
int Javascript_interpreter::interpret_file(std::string file_name) {
  std::string str_command = "node ";
  str_command += this->files_path + file_name;
  const char *command = str_command.c_str();
  system(command);
  return 0;
};

int Javascript_interpreter::interpret_code(std::string code) {
  std::ofstream aux_file;
  aux_file.open(this->files_path + "aux_file.js");
  if (!aux_file) return 1;
  aux_file << code;
  aux_file.close();
  this->interpret_file("aux_file.js");
  return 0;
};

void Javascript_interpreter::print_path() {
  std::cout << files_path;
};