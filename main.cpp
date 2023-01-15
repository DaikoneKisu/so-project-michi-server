#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>

#include <boost/asio/thread_pool.hpp>
#include "javascript interpreter/javascript_interpreter.h"
#include "http/server.hpp"

int main(int argc, char* argv[]) {
    boost::asio::thread_pool thread_pool {std::thread::hardware_concurrency()};

    bool sent_response = {false};
    http::Server http_server {thread_pool.get_executor()};
    http_server
        .on(http::Verb::post, "/cpp",
            [&sent_response](http::Request& request, http::Response& response, http::ResponseSender send_response) {

                std::ofstream code_to_compile {"code-to-compile.cpp"};
                code_to_compile << request.body();
                code_to_compile.close();

                std::system("g++ -std=c++2a -ocode-to-run code-to-compile.cpp -Iboost_1_81_0 -lwsock32 -lws2_32");
                //std::system("g++ -std=c++2a code-to-compile.cpp -o code-to-run");
                std::system("PowerShell -Command \"./code-to-run 127.0.0.1 8080 > code-output.txt \"");
                
                std::ifstream code_output {"code-output.txt"};

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
            [&sent_response](http::Request& request, http::Response& response, http::ResponseSender send_response) {
                
                std::ofstream code_to_interpret {"code-to-interpret.js"};
                code_to_interpret << request.body();
                code_to_interpret.close();

                std::system("PowerShell -Command \"node code-to-interpret.js > code-output.txt\"");
                
                std::ifstream code_output {"code-output.txt"};

                response.version(11);
                response.result(http::Status::created);
                response.set(http::Field::server, "Michi-Server");
                response.set(http::Field::content_type, "text/plain; charset=utf-8");
                response.body() = std::string {std::istreambuf_iterator {code_output}, {}};
                response.set(http::Field::content_length, std::to_string(response.body().size()));
                response.keep_alive(request.keep_alive());
                send_response();
                code_output.close();
            }
        )
        .on(http::Verb::get, ".*",
            [&sent_response](http::Request& request, http::Response& response, http::ResponseSender send_response) {
                if (sent_response) {
                    return;
                }
                response.version(11);
                response.result(http::Status::ok);
                response.set(http::Field::server, "Michi-Server");
                response.body() = "=^._.^=";
                response.prepare_payload();
                response.keep_alive(request.keep_alive());
                send_response();
            }
        )
        .async_start(argv[1], std::stoi(argv[2]));
    
    thread_pool.join();
    return 0;
}