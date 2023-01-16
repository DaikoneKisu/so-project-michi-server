#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>

#include <boost/asio/thread_pool.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "http/server.hpp"

using namespace std::string_literals;

std::string generate_random_name() {
    boost::uuids::random_generator random_uuid_generator;
    boost::uuids::uuid uuid = random_uuid_generator();
    return boost::uuids::to_string(uuid);
}

int main(int argc, char* argv[]) {
    boost::asio::thread_pool thread_pool {std::thread::hardware_concurrency()};

    http::Server http_server {thread_pool.get_executor()};
    http_server
        .on(http::Verb::post, "/cpp",
            [](http::Request& request, http::Response& response, http::ResponseSender send_response) {
                // ...
            }
        )
        .on(http::Verb::post, "/js",
            [](http::Request& request, http::Response& response, http::ResponseSender send_response) {
                // ...
            }
        )
        .async_start(argv[1], std::stoi(argv[2]));
    
    
    thread_pool.join();
    return 0;
}