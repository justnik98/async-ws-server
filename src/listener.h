//
// Created by justnik on 06.03.2021.
//

#ifndef ASYNC_SERVER_LISTENER_H
#define ASYNC_SERVER_LISTENER_H

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
#include "session.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener
        : public boost::asio::coroutine, public std::enable_shared_from_this<listener> {
    net::io_context &ioc_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;

public:
    listener(
            net::io_context &ioc,
            const tcp::endpoint &endpoint);

    // Start accepting incoming connections
    void run() {
        loop();
    }

private:
    void loop(beast::error_code ec = {});
};


#endif //ASYNC_SERVER_LISTENER_H
