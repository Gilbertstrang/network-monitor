#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/system/error_code.hpp>

#include <iomanip>
#include <iostream>
#include <string>

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;

void Log(const std::string& where, boost::system::error_code ec) {
    std::cerr << "[" << std::setw(20) << where << "] "
              << (ec ? "Error: " : "OK")
              << (ec ? ec.message() : "")
              << std::endl;
}

void OnReceive(
    boost::beast::flat_buffer& rbuffer,
    const boost::system::error_code& ec
) {
    if (ec) {
        Log("OnReceive", ec);
        return;
    }

    std::cout << "ECHO: "
              << boost::beast::make_printable(rbuffer.data())
              << std::endl;

}

void OnSend(
    websocket::stream<boost::beast::tcp_stream>& ws,
    boost::beast::flat_buffer& rBuffer,
    const boost::system::error_code& ec
) {
    if (ec) {
        Log("OnSend", ec);
        return;
    }

    ws.async_read(rBuffer, [&rBuffer](auto ec, auto nBytesRead) {
        OnReceive(rBuffer, ec);
    });

}

void OnHandshake(
    websocket::stream<boost::beast::tcp_stream>& ws,
    const boost::asio::const_buffer& wBuffer,
    boost::beast::flat_buffer& rBuffer,
    const boost::system::error_code& ec
) {
    if (ec) {
        Log("OnHandshake", ec);
        return;
    }

    ws.text(true);

    ws.async_write(wBuffer, [&ws, &rBuffer](auto ec, auto nBytesWritten) {
        OnSend(ws, rBuffer, ec);
    });
}

void OnConnect(
    websocket::stream<boost::beast::tcp_stream>& ws,
    const std::string& url,
    const std::string& endpoint,
    const boost::asio::const_buffer& wBuffer,
    boost::beast::flat_buffer& rBuffer,
    const boost::system::error_code& ec
) {
    if (ec) {
        Log("OnConnect", ec);
        return;
    }

    ws.async_handshake(url, endpoint, [&ws, &wBuffer, &rBuffer](auto ec) {
        OnHandshake(ws, wBuffer, rBuffer, ec);
    });
}

void OnResolve(
    websocket::stream<boost::beast::tcp_stream>& ws,
    const std::string& url,
    const std::string& endpoint,
    const boost::asio::const_buffer& wBuffer,
    boost::beast::flat_buffer& rBuffer,
    const boost::system::error_code& ec,
    tcp::resolver::iterator resolverIt
) {
    if (ec) {
        Log("OnResolve", ec);
        return;
    }

    ws.next_layer().async_connect(*resolverIt, [&ws, &url, &endpoint, &wBuffer, &rBuffer](auto ec) {
        OnConnect(ws, url, endpoint, wBuffer, rBuffer, ec);
    });
}


int main()
{

    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string port {"80"};
    const std::string endpoint {"/echo"};
    const std::string message {"hello there"};


    boost::asio::io_context ioc {};

    websocket::stream<boost::beast::tcp_stream> ws {ioc};
    boost::asio::const_buffer wBuffer {message.c_str(), message.size()};
    boost::beast::flat_buffer rBuffer {};

    tcp::resolver resolver {ioc};
    resolver.async_resolve(url, port, [&ws, &url, &endpoint, &wBuffer, &rBuffer](auto ec, auto resolverIt) {
        OnResolve(ws, url, endpoint, wBuffer, rBuffer, ec, resolverIt);
    });
    
    ioc.run();
    
    if ((boost::beast::buffers_to_string(rBuffer.data()) != message)) {
        return -1;
    }
    
    return 0;
}