//
// Created by justnik on 06.03.2021.
//

#include "listener.h"

listener::listener(net::io_context &ioc, const tcp::endpoint &endpoint)
        : ioc_(ioc), acceptor_(net::make_strand(ioc)), socket_(net::make_strand(ioc)) {
    beast::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        fail(ec, "open");
        return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
        fail(ec, "set_option");
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if (ec) {
        fail(ec, "bind");
        return;
    }

    // Start listening for connections
    acceptor_.listen(
            net::socket_base::max_listen_connections, ec);
    if (ec) {
        fail(ec, "listen");
        return;
    }
}

void listener::loop(beast::error_code ec) {
    reenter(*this) {
                    for (;;) {
                        yield
                                                                acceptor_.async_accept(
                                                                        socket_,
                                                                        [capture0 = shared_from_this()](auto &&PH1) {
                                                                            capture0->loop(
                                                                                    std::forward<decltype(PH1)>(PH1));
                                                                        });
                        if (ec) {
                            fail(ec, "accept");
                        } else {
                            // Create the session and run it
                            std::make_shared<session>(std::move(socket_))->run();
                        }

                        // Make sure each session gets its own strand
                        socket_ = tcp::socket(net::make_strand(ioc_));
                    }
                }
}
