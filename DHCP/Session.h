#pragma once
#include <stdint.h>
#include <vector>
#include <boost/asio.hpp>

namespace bric::Networking::DHCP
{
    namespace asio = boost::asio;
    namespace ip = asio::ip;
    using protocol = ip::udp;
    class Session : public asio::io_context
    {
    private:
       ip::address hostAddr;
       ip::address dnsAddr;
       ip::address gatewayAddr;
       ip::address minAddr;
       ip::address maxAddr;
       std::string hostName;

        
    public:
        static constexpr ip::port_type serverPort = 67;
        static constexpr ip::port_type clientPort = 68;
        Session(const char* hostAddr,const char* dnsAddr,const char* gatewayAddr,const char* minAddr,const char* maxAddr);
        void exec_listen();
        ~Session();
    };
} // namespace bric::Networking::DHCP
