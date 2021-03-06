//
// Created by justnik on 06.03.2021.
//

#include <string>
#include <sstream>
#include "pipeline.h"
#include "session.h"


session::session(tcp::socket socket)
        : ws_(std::move(socket)) {
}

void session::run() {
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.
    net::dispatch(ws_.get_executor(),
                  beast::bind_front_handler(&session::loop,
                                            shared_from_this(),
                                            beast::error_code{},
                                            0));
}

void session::loop(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    std::string str;
    reenter(*this) {
                    // Set suggested timeout settings for the websocket
                    ws_.set_option(
                            websocket::stream_base::timeout::suggested(
                                    beast::role_type::server));

                    // Set a decorator to change the Server of the handshake
                    ws_.set_option(websocket::stream_base::decorator(
                            [](websocket::response_type &res) {
                                res.set(http::field::server,
                                        std::string(BOOST_BEAST_VERSION_STRING) +
                                        " websocket-server-stackless");
                            }));

                    // Accept the websocket handshake
                    yield
                                                            ws_.async_accept(
                                                                    [capture0 = shared_from_this()](auto &&PH1) {
                                                                        capture0->loop(std::forward<decltype(PH1)>(PH1),
                                                                                       0);
                                                                    });
                    if (ec)
                        return fail(ec, "accept");

                    for (;;) {
                        // Read a message into our buffer
                        yield
                                                                ws_.async_read(
                                                                        buffer_,
                                                                        [capture0 = shared_from_this()](auto &&PH1,
                                                                                                        auto &&PH2) {
                                                                            capture0->loop(
                                                                                    std::forward<decltype(PH1)>(PH1),
                                                                                    std::forward<decltype(PH2)>(PH2));
                                                                        });
                        if (ec == websocket::error::closed) {
                            // This indicates that the session was closed
                            return;
                        }
                        if (ec)
                            fail(ec, "read");

                        // Echo the message
                        ws_.text(ws_.got_text());
                        str = (boost::beast::buffers_to_string(buffer_.data()));
                        std::cout << str << std::endl;
                        runPipeline(str);
                        std::this_thread::sleep_for(std::chrono::seconds(10));

                        yield
                                                                ws_.async_write(
                                                                        buffer_.data(),
                                                                        [capture0 = shared_from_this()](auto &&PH1,
                                                                                                        auto &&PH2) {
                                                                            capture0->loop(
                                                                                    std::forward<decltype(PH1)>(PH1),
                                                                                    std::forward<decltype(PH2)>(PH2));
                                                                        });
                        if (ec)
                            return fail(ec, "write");

                        // Clear the buffer
                        buffer_.consume(buffer_.size());
                    }
                }
}
