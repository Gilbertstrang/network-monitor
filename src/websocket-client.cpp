#include <network-monitor/websocket-client.h>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/system/error_code.hpp>
#include <string>
#include <functional>
#include <iomanip>
#include <chrono>
#include <iostream>


using NetworkMonitor::WebSocketClient;
using tcp = boost::asio::ip::tcp;

namespace websocket = boost::beast::websocket;

static void Log(const std::string& where, boost::system::error_code ec) {
    std::cerr << "[" << std::setw(20) << where << "] "
              << (ec ? "Error: " : "OK")
              << (ec ? ec.message() : "")
              << std::endl;
}

WebSocketClient::WebSocketClient(
    const std::string& url,
    const std::string& endpoint,
    const std::string& port,
    boost::asio::io_context& ioc
) : url_ {url},
    endpoint_ {endpoint},
    port_ {port},
    resolver_ {boost::asio::make_strand(ioc)},
    ws_ {boost::asio::make_strand(ioc)}
{
}

WebSocketClient::~WebSocketClient() = default;


void WebSocketClient::Connect(
    std::function<void (boost::system::error_code)> onConnect,
    std::function<void (boost::system::error_code, std::string&&)> onMessage,
    std::function<void (boost::system::error_code)> onDisconnect 
) {
    onConnect_ = onConnect;
    onMessage_ = onMessage;
    onDisconnect_ = onDisconnect;

    closed_ = false;
    resolver_.async_resolve(url_, port_, [this](auto ec, auto resolverIt) {
        OnResolve(ec, resolverIt);
    });

}

void WebSocketClient::Send(
    const std::string& message,
    std::function<void (boost::system::error_code)> onSend
) {
    ws_.async_write(boost::asio::buffer(message), [onSend](auto ec, auto) {
        if (onSend) {
            onSend(ec);
        }
    });
}

void WebSocketClient::Close(
    std::function<void (boost::system::error_code)> onClose
) {
    closed_ = true;
    ws_.async_close(websocket::close_code::none, [onClose](auto ec) {
        if (onClose) {
            onClose(ec);
        }
    });
}

void WebSocketClient::OnResolve(
    const boost::system::error_code& ec,
    tcp::resolver::iterator resolverIt
) {
    if (ec) {
        Log("OnResolve", ec);
        if (onConnect_) {
            onConnect_(ec);
        }
        return;
    }

    ws_.next_layer().expires_after(std::chrono::seconds(5));

    ws_.next_layer().async_connect(*resolverIt, [this](auto ec) {
        OnConnect(ec);
    });
}

void WebSocketClient::OnConnect(
    const boost::system::error_code& ec
) {
    if (ec) {
        Log("OnConnect", ec);
        if (onConnect_) {
            onConnect_(ec);
        }
        return;
    }

    //since we connected TCP socket now, reset timeout to Boost's suggestion
    ws_.next_layer().expires_never();
    ws_.set_option(websocket::stream_base::timeout::suggested(boost::beast::role_type::client));

    //handshake attempt
    ws_.async_handshake(url_, endpoint_, [this](auto ec){
        OnHandshake(ec);
    });

}

void WebSocketClient::OnHandshake(
    const boost::system::error_code& ec
) {
    if (ec) {
        Log("OnHandshake", ec);
        if (onConnect_) {
            onConnect_(ec);
        }
        return;
    }

    ws_.text(true); //exchange message int text format

    ListenToIncomingMessage(ec); //since we are connected now, set a recursive async listener to receive msg

    //dispatch the user callback
    if (onConnect_) {
        onConnect_(ec);
    }

}

void WebSocketClient::ListenToIncomingMessage(
    const boost::system::error_code& ec
) {
    //stop processing messages if connection aborted
    if (ec == boost::asio::error::operation_aborted) {
        if (onDisconnect_ && ! closed_) { //check closed flag to avoid notifying user twice
            onDisconnect_(ec);
        }
        return;
    }

    //read message async. on successful read process message
    //and recursively called this again for next messages
    ws_.async_read(rBuffer_, [this](auto ec, auto nBytes) {
        OnRead(ec, nBytes);
        ListenToIncomingMessage(ec);
    });


}

void WebSocketClient::OnRead(
    const boost::system::error_code& ec,
    size_t nBytes
) {
    //ignore messages failed to read lmao
    if (ec) {
        return;
    }

    std::string message {boost::beast::buffers_to_string(rBuffer_.data())};
    rBuffer_.consume(nBytes);
    if (onMessage_) {
        onMessage_(ec, std::move(message));
    }
}