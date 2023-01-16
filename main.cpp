#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>

#include <boost/asio/thread_pool.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "javascript interpreter/javascript_interpreter.h"
#include "http/server.hpp"

using namespace std::string_literals;

std::string generate_random_name() {
    boost::uuids::random_generator random_uuid_generator;
    boost::uuids::uuid uuid = random_uuid_generator();
    return boost::uuids::to_string(uuid);
}

int main(int argc, char* argv[]) {
    std::system("mkdir .\\.tmp");

    boost::asio::thread_pool thread_pool {std::thread::hardware_concurrency()};

    http::Server http_server {thread_pool.get_executor()};
    http_server
        .on(http::Verb::post, "/cpp",
            [](http::Request& request, http::Response& response, http::ResponseSender send_response) {
                std::string code_to_compile_filename = ".tmp\\"s + generate_random_name() + ".cpp"s;
                std::ofstream code_to_compile {code_to_compile_filename};
                code_to_compile << request.body();
                code_to_compile.close();

                std::string executable_filename = ".tmp\\"s + generate_random_name() + ".exe"s;
                std::string code_output_filename = ".tmp\\"s + generate_random_name() + ".txt"s;

                std::system(("g++ -std=c++2a -o "s + executable_filename + " "s + code_to_compile_filename).data());
                std::system(("PowerShell -Command \"./"s + executable_filename + " > " + code_output_filename + "\""s).data());
                
                std::ifstream code_output {code_output_filename};

                response.version(11);
                response.result(http::Status::created);
                response.set(http::Field::server, "Michi-Server");
                response.set(http::Field::content_type, "text/plain; charset=utf-16");
                response.body() = std::string {std::istreambuf_iterator {code_output}, {}};
                response.set(http::Field::content_length, std::to_string(response.body().size()));
                response.keep_alive(request.keep_alive());
                send_response();

                code_output.close();
            }
        )
        .on(http::Verb::post, "/js",
            [](http::Request& request, http::Response& response, http::ResponseSender send_response) {
                std::string code_to_interpret_filename = ".tmp\\"s + generate_random_name() + ".js"s;
                std::ofstream code_to_interpret {code_to_interpret_filename};
                code_to_interpret << request.body();
                code_to_interpret.close();

                std::string code_output_filename = ".tmp\\"s + generate_random_name() + ".txt"s;

                std::system(("PowerShell -Command \"node "s + code_to_interpret_filename + " > " + code_output_filename + "\""s).data());
                
                std::ifstream code_output {code_output_filename};

                response.version(11);
                response.result(http::Status::created);
                response.set(http::Field::server, "Michi-Server");
                response.set(http::Field::content_type, "text/plain; charset=utf-16");
                response.body() = std::string {std::istreambuf_iterator {code_output}, {}};
                response.set(http::Field::content_length, std::to_string(response.body().size()));
                response.keep_alive(request.keep_alive());
                send_response();

                code_output.close();
            }
        )
        .async_start(argv[1], std::stoi(argv[2]));
    
    thread_pool.join();
    return 0;
}