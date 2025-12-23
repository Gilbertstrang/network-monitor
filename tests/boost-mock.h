#ifndef NETWORK_MONITOR_TESTS_BOOST_MOCK_H
#define NETWORK_MONITOR_TESTS_BOOST_MOCK_H

#include <network-monitor/websocket-client.h>

#include <boost/asio.hpp>
#include <boost/utility/string_view.hpp>


namespace NetworkMonitor {


/*! \brief mocks DNS resolver from Asio
*/
class MockResolver {
public:
    /*! \brief Use this static member in a test to set the error code returned
     *         by async_resolve.
    
    */
    static boost::system::error_code resolveEc;

    /*! \brief Mock for the resolver constructor
     */
    template<typename ExecutionContext>
    explicit MockResolver(ExecutionContext&& ctx) : ctx_ {ctx} {

    }
    
     /*! \brief Mock for resolver::async_resolve
     */
    template<typename ResolveHandler>
    void async_resolve(
        boost::string_view host,
        boost::string_view service,
        ResolveHandler&& handler
    ) {
        using resolver = boost::asio::ip::tcp::resolver;
        
        return boost::asio::async_initiate<
            ResolveHandler, void (const boost::system::error_code&, resolver::results_type)>(
                [](auto&& handler, auto resolver) {
                    if (MockResolver::resolveEc) {
                        boost::asio::post(
                            resolver->ctx_, 
                            boost::beast::bind_handler(std::move(handler), MockResolver::resolveEc, resolver::results_type {}));
                    } else {

                    }
                },
                handler, this
            );
    }
private:

    boost::asio::strand<boost::asio::io_context::executor_type> ctx_;

};

// Out-of-line static member initialization
inline boost::system::error_code MockResolver::resolveEc {};


/*! \brief Type alias for the mocked WebSocketClient.
 *
 *  For now we only mock the DNS resolver.
 */
using TestWebSocketClient = WebSocketClient<
    MockResolver, 
    boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>>>;



} // namespace NetworkMonitor

#endif