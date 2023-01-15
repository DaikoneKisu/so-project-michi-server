#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <functional>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/write.hpp>

namespace http {

using Verb = boost::beast::http::verb;

using Status = boost::beast::http::status;

using Field = boost::beast::http::field;

using Request = boost::beast::http::request<boost::beast::http::string_body>;

using Response = boost::beast::http::response<boost::beast::http::string_body>;

using ResponseSender = std::function<void()>;

using RequestHandler = std::function<void(Request&, Response&, ResponseSender)>;

class RequestHandlersDispatcher {
public:
    void register_handler(Verb request_verb, const std::string& path_pattern, RequestHandler request_handler) {
        _handlers[request_verb].emplace_back(std::make_pair(path_pattern, std::move(request_handler)));
    }

    void dispatch_handlers(Request& request, Response& response, ResponseSender response_sender) const {
        if (_handlers.find(request.method()) == _handlers.end()) {
            return;
        }
        for (auto&& [path_pattern, request_handler] : _handlers.at(request.method())) {
            if (std::regex_match(request.target().data(), std::regex{path_pattern})) {
                request_handler(request, response, response_sender);
            }
        }
    }

private:
    std::unordered_map<Verb, std::vector<std::pair<std::string, RequestHandler>>> _handlers;
};

class Session {
public:
    Session(boost::asio::ip::tcp::socket server_socket, const RequestHandlersDispatcher& request_handlers_dispatcher)
        : _server_socket {std::move(server_socket)}
        , _reading_buffer {1024} // 1 KiB buffer
        , _request_handlers_dispatcher {request_handlers_dispatcher}
    {}

    void async_start() {
        boost::asio::post(
            [&] {
                std::cout << "Session started - client located at " << _server_socket.socket().remote_endpoint() << '\n';
                while (true) {
                    boost::beast::error_code error_code;
                    Request request {};
                    boost::beast::http::read(_server_socket, _reading_buffer, request, error_code);
                    if (error_code) {
                        stop();
                        return;
                    }
                    
                    std::cout << "Request received (" << request.method_string() << " " << request.target()
                        << ") - client located at " << _server_socket.socket().remote_endpoint() << '\n';

                    std::cout << request << '\n';

                    Response response {};
                    _request_handlers_dispatcher.dispatch_handlers(request, response, [&]() {
                        boost::beast::http::write(_server_socket, response);

                        std::cout << "Response sent (status " << response.result_int()
                            << ") - client located at " << _server_socket.socket().remote_endpoint() << '\n';
                        
                        std::cout << response << '\n';
                    });
                    

                    if (!request.keep_alive() || !response.keep_alive()) {
                        stop();
                        return;
                    }
                }
            }
        );
    }

private:
    void stop() {
        _server_socket.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        std::cout << "Session finished - client located at " << _server_socket.socket().remote_endpoint() << '\n';
    }

    boost::beast::tcp_stream _server_socket;
    boost::beast::flat_buffer _reading_buffer;
    const RequestHandlersDispatcher& _request_handlers_dispatcher;
};

template <typename Executor>
class Server {
public:
    Server(Executor executor)
        : _executor {executor}
        , _acceptor_socket {_executor}
        , _request_handlers_dispatcher {}
        , _active_sessions {}
    {}

    Server& on(Verb request_verb, const std::string& path_pattern, RequestHandler request_handler) {
        _request_handlers_dispatcher.register_handler(request_verb, path_pattern, std::move(request_handler));
        return *this;
    }

    void async_start(const std::string& ip_address, unsigned short int port) {
        boost::asio::ip::tcp::endpoint endpoint {boost::asio::ip::make_address(ip_address), port};
        _acceptor_socket.open(endpoint.protocol());
        _acceptor_socket.set_option(boost::asio::ip::tcp::socket::reuse_address{true});
        _acceptor_socket.bind(endpoint);
        _acceptor_socket.listen();
        async_accept_connection_requests();

        std::cout << "Server started - listening for connection requests at " << endpoint << '\n';
    }

private:
    void async_accept_connection_requests() {
        _acceptor_socket.async_accept(
            [&](const boost::system::error_code&, boost::asio::ip::tcp::socket server_socket) {
                auto new_session = std::make_unique<Session>(std::move(server_socket), _request_handlers_dispatcher);
                new_session->async_start();
                _active_sessions.emplace(std::move(new_session));

                async_accept_connection_requests();
            });
    }

    Executor _executor;
    boost::asio::ip::tcp::acceptor _acceptor_socket;
    RequestHandlersDispatcher _request_handlers_dispatcher;
    std::unordered_set<std::unique_ptr<Session>> _active_sessions;
};

} // namespace http

#endif