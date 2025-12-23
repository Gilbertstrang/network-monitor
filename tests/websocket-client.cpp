#include "boost-mock.h"

#include <network-monitor/websocket-client.h>

#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <string>
#include <filesystem>

using NetworkMonitor::BoostWebSocketClient;
using NetworkMonitor::MockResolver;
using NetworkMonitor::TestWebSocketClient;

// init mock properties
struct WebSocketClientTestFixture {
    WebSocketClientTestFixture() {
        MockResolver::resolveEc = {};
    }
};

using timeout = boost::unit_test::timeout; //timeout for connections that slow or that doesnt go anywhere..

BOOST_AUTO_TEST_SUITE(network_monitor);

BOOST_AUTO_TEST_SUITE(class_WebSocketClient);
BOOST_AUTO_TEST_CASE(cacert_pem) {
    BOOST_CHECK(std::filesystem::exists(TESTS_CACERT_PEM));
}
BOOST_FIXTURE_TEST_SUITE(Connect, WebSocketClientTestFixture);

BOOST_AUTO_TEST_CASE(fail_resolve, *timeout {1}) {
    //mock
    const std::string url {"some.echo-server.com"};
    const std::string port {"443"};
    const std::string endpoint {"/"};

    boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};

    ctx.load_verify_file(TESTS_CACERT_PEM);

    boost::asio::io_context ioc {};
    
    MockResolver::resolveEc = boost::asio::error::host_not_found;

    TestWebSocketClient client {url, endpoint, port, ioc, ctx};

    bool calledOnConnect {false};
    auto onConnect {[&calledOnConnect](auto ec) {
        calledOnConnect = true;
        BOOST_CHECK_EQUAL(ec, boost::asio::error::host_not_found);
    }};

    client.Connect(onConnect);
    ioc.run();

    BOOST_CHECK(calledOnConnect);

}

BOOST_AUTO_TEST_SUITE_END(); //connect

BOOST_AUTO_TEST_SUITE(live);


BOOST_AUTO_TEST_CASE(echo, *timeout {20}) {
    const std::string url {"ltnm.learncppthroughprojects.com"};
    const std::string port {"443"};
    const std::string endpoint {"/echo"};
    const std::string message {"hello world"};

    boost::asio::ssl::context ctx {boost::asio::ssl::context::tlsv12_client}; //TLS context
    ctx.load_verify_file(TESTS_CACERT_PEM);

    boost::asio::io_context ioc {};

    BoostWebSocketClient client {url, endpoint, port, ioc, ctx}; //tested class

    //flags
    bool connected {false};
    bool messageSent {false};
    bool messageReceived {false};
    bool disconnected {false};
    std::string echo {};

    auto onSend {[&messageSent](auto ec) {
        messageSent = !ec;
    }};

    auto onConnect {[&client, &connected, &onSend, &message](auto ec){
        connected = !ec;

        if (!ec) {
            client.Send(message, onSend);
        }
    }};

    auto onClose {[&disconnected](auto ec) {
        disconnected = !ec;
    }};


    auto onReceive {[&client, &onClose, &messageReceived, &echo](auto ec, auto received){
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

bool CheckResponse(const std::string& response) {

    bool ok {true};
    ok &= response.find("ERROR") != std::string::npos;
    ok &= response.find("ValidationInvalidAuth") != std::string::npos;
    return ok;
}


BOOST_AUTO_TEST_CASE(network_events, *timeout {3}) {
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
BOOST_AUTO_TEST_SUITE_END(); //live
BOOST_AUTO_TEST_SUITE_END(); //class_websocketclient
BOOST_AUTO_TEST_SUITE_END(); //network_monitor
