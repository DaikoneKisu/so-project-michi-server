#include <thread>

#include <boost/asio/thread_pool.hpp>

#include "http/server.hpp"

int main() {
    boost::asio::thread_pool thread_pool {std::thread::hardware_concurrency()};

    bool sent_response = {false};
    http::Server http_server {thread_pool.get_executor()};
    http_server
        .on(http::Verb::get, "/tupapaesmimama",
            [&sent_response](http::Request& request, http::Response& response, http::ResponseSender send_response) {
                response.version(11);
                response.result(http::Status::ok);
                response.set(http::Field::server, "Michi-Server");
                response.body() = "=^._.^=";
                response.prepare_payload();
                response.keep_alive(request.keep_alive());
                send_response();
                sent_response = true;
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
                response.body() = "Por aqui no es gei";
                response.prepare_payload();
                response.keep_alive(request.keep_alive());
                send_response();
            }
        )
        .async_start("127.0.0.1", 80);
    
    thread_pool.join();
    return 0;
}