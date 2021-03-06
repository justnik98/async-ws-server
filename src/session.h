//
// Created by justnik on 06.03.2021.
//

#ifndef ASYNC_SERVER_SESSION_H
#define ASYNC_SERVER_SESSION_H

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/dispatch.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// Report a failure
void inline fail(beast::error_code ec, char const *what) {
    std::cerr << what << ": " << ec.message() << "\n";
}
// Echoes back all received WebSocket messages
class session
        : public boost::asio::coroutine, public std::enable_shared_from_this<session> {
    websocket::stream <beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;

public:
    // Take ownership of the socket
    explicit session(tcp::socket socket);

    // Start the asynchronous operation
    void run();

#include <boost/asio/yield.hpp>

    void loop(beast::error_code ec, std::size_t bytes_transferred);
};

#endif //ASYNC_SERVER_SESSION_H
