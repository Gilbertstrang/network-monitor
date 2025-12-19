#include <network-monitor/websocket-client.h>
#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <string>
#include <filesystem>

using NetworkMonitor::BoostWebSocketClient;


BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_CASE(cacert_pem) {
    BOOST_CHECK(std::filesystem::exists(TESTS_CACERT_PEM));
}

BOOST_AUTO_TEST_CASE(class_WebSocketClient) {
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string port {"443"};
    const std::string endpoint {"/echo"};
    const std::string message {"hello there"};
    boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};

    ctx.load_verify_file(TESTS_CACERT_PEM);

    boost::asio::io_context ioc {};
    
    BoostWebSocketClient client {url, endpoint, port, ioc, ctx};

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

BOOST_AUTO_TEST_CASE(send_stomp_frame) {
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string port {"443"};
    const std::string endpoint {"/network-events"};

    //stomp frame
    const std::string username {"riker"};
    const std::string password {"picard"};
    std::stringstream ss {};
    ss << "STOMP" << std::endl
       << "accept-version:1.2" << std::endl
       << "host:transportforlondon.com" << std::endl
       << "login:" << username << std::endl
       << "passcode:" << password << std::endl
       << std::endl // Headers need to be followed by a blank line.
       << '\0'; // The body (even if absent) must be followed by a NULL octet.
    const std::string message {ss.str()};

    //tls 
    boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};
    ctx.load_verify_file(TESTS_CACERT_PEM);

    boost::asio::io_context ioc {};
    
    BoostWebSocketClient client {url, endpoint, port, ioc, ctx};

    bool connected {false};
    bool messageSent {false};
    bool messageReceived {false};
    bool disconnected {false};
    std::string serverError {};

    auto CheckResponse {[](const std::string& response) {
        // We do not parse the whole message. We only check that it contains some
        // expected items.
        bool ok {true};
        ok &= response.find("ERROR") != std::string::npos;
        ok &= response.find("ValidationInvalidAuth") != std::string::npos;
        return ok;
    }};


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

    auto onReceive {[&client, &onClose, &messageReceived, &serverError](auto ec, auto received) {
        messageReceived = !ec;
        serverError = std::move(received);
        client.Close(onClose);
    }};
    
    
    client.Connect(onConnect, onReceive);
    ioc.run();

    BOOST_CHECK(connected);
    BOOST_CHECK(messageSent);
    BOOST_CHECK(messageReceived);
    BOOST_CHECK(CheckResponse(serverError));
    BOOST_CHECK(disconnected);


}

BOOST_AUTO_TEST_SUITE_END();
