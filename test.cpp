#include <fstream>
#include <cstdlib>
#include <iostream>
#include <iterator>

int main() {
  std::system("PowerShell -Command \"node code-to-interpret.js > code-output.txt\"");
  std::ifstream code_output {"code-output.txt"};
  std::cout << std::string{std::istreambuf_iterator{code_output}, {}};
  return 0;
}