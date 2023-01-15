#ifndef javascript_interpreter_h
#define javascript_interpreter_h

#include <iostream>

class Javascript_interpreter {
  public:
    //Constructor Declaration
    Javascript_interpreter(std::string files_path);
    //Methods Declaration
    int interpret_file(std::string file_name);
    int interpret_code(std::string code);
    void print_path();
  private:
    std::string files_path;
};

#endif