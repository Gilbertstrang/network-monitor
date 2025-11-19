#include <network-monitor/websocket-client.h>
#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <string>

using NetworkMonitor::WebSocketClient;


BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_CASE(class_WebSocketClient) {
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string port {"80"};
    const std::string endpoint {"/echo"};
    const std::string message {"hello there"};


    boost::asio::io_context ioc {};

    WebSocketClient client {url, endpoint, port, ioc};

    bool connected {false};
    bool messageSent {false};
    bool messageReceived {false};
    bool disconnected {false};
    std::string echo {};

    auto onSend {[&messageSent](auto ec) {
        messageSent = !ec;
    }};

    auto onConnect {[&client, &connected, &onSend, &message](auto ec) {
        connected = !ec;
        if (!ec) client.Send(message, onSend);
    }};

    auto onClose {[&disconnected](auto ec) {
        disconnected = !ec;
    }};

    auto onReceive {[&client, &onClose, &messageReceived, &echo](auto ec, auto received) {
        messageReceived = !ec;
        echo = std::move(received);
        client.Close(onClose);
    }};

    client.Connect(onConnect, onReceive);
    ioc.run();

    BOOST_CHECK(connected);
    BOOST_CHECK(messageSent);
    BOOST_CHECK(messageReceived);
    BOOST_CHECK(disconnected);
    BOOST_CHECK_EQUAL(message, echo);


}

BOOST_AUTO_TEST_SUITE_END();
