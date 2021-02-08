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
void
fail(beast::error_code ec, char const *what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

// Echoes back all received WebSocket messages
class session
        : public boost::asio::coroutine, public std::enable_shared_from_this<session> {
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;

public:
    // Take ownership of the socket
    explicit
    session(tcp::socket socket)
            : ws_(std::move(socket)) {
    }

    // Start the asynchronous operation
    void
    run() {
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

#include <boost/asio/yield.hpp>

    void
    loop(
            beast::error_code ec,
            std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);
        reenter(*this)
        {
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
                        capture0->loop(std::forward<decltype(PH1)>(PH1), 0);
                    });
            if (ec)
                return fail(ec, "accept");

            for (;;) {
                // Read a message into our buffer
                yield
                ws_.async_read(
                        buffer_,
                        [capture0 = shared_from_this()](auto &&PH1, auto &&PH2) {
                            capture0->loop(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
                        });
                if (ec == websocket::error::closed) {
                    // This indicates that the session was closed
                    return;
                }
                if (ec)
                    fail(ec, "read");

                // Echo the message
                ws_.text(ws_.got_text());
                std::cout << boost::beast::buffers_to_string(buffer_.data()) << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(10));

                yield
                ws_.async_write(
                        buffer_.data(),
                        [capture0 = shared_from_this()](auto &&PH1, auto &&PH2) {
                            capture0->loop(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
                        });
                if (ec)
                    return fail(ec, "write");

                // Clear the buffer
                buffer_.consume(buffer_.size());
            }
        }
    }
};

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
            const tcp::endpoint &endpoint)
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

    // Start accepting incoming connections
    void
    run() {
        loop();
    }

private:


    void
    loop(beast::error_code ec = {}) {
        reenter(*this)
        {
            for (;;) {
                yield
                acceptor_.async_accept(
                        socket_,
                        [capture0 = shared_from_this()](auto &&PH1) {
                            capture0->loop(std::forward<decltype(PH1)>(PH1));
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
};

//------------------------------------------------------------------------------

int main(int argc, char *argv[]) {
    // Check command line arguments.
    if (argc != 4) {
        std::cerr <<
                  "Usage: websocket-server-stackless <address> <port> <threads>\n" <<
                  "Example:\n" <<
                  "    websocket-server-stackless 0.0.0.0 8080 1\n";
        return EXIT_FAILURE;
    }
    auto const address = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const threads = std::max<int>(1, std::atoi(argv[3]));

    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<listener>(ioc, tcp::endpoint{address, port})->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back(
                [&ioc] {
                    ioc.run();
                });
    ioc.run();

    return EXIT_SUCCESS;
}