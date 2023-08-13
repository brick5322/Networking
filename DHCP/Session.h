#pragma once
#include <stdint.h>
#include <vector>
#include <boost/asio.hpp>
#include <chrono>

namespace bric::Networking::DHCP
{
    namespace asio = boost::asio;
    using time_point = std::chrono::system_clock::time_point;

    struct IPInfo 
    {
        time_point releaseTime;
	    basic_vector clientIdentifier;
    };

    class Session : public asio::io_context
    {
    private:
       asio::ip::address hostAddr;
       asio::ip::address dnsAddr;
       asio::ip::address gatewayAddr;
       asio::ip::address subnetMask;
       asio::ip::address minAddr;
       asio::ip::address maxAddr;
       std::string hostName;
       std::vector<IPInfo> ipPool;
       std::map<basic_vector,asio::ip::address> clientIdentifier2Ip;
       std::chrono::seconds leaseTime;
       std::chrono::seconds offerKeepTime;

       IPInfo& infoAt(asio::ip::address addr);
       asio::ip::address getAddressAt(int index);
       bool addressAvaliable(uint32_t index);

       Message setReply(const Message& request,const asio::ip::address& offered);
        
    public:
        static constexpr std::chrono::seconds maxLeasetime = std::chrono::seconds(60*60*24*365);
        static constexpr asio::ip::port_type serverPort = 67;
        static constexpr asio::ip::port_type clientPort = 68;
        Session(const char* hostAddr,
                const char* dnsAddr,
                const char* gatewayAddr,
                const char* minAddr,
                const char* maxAddr,
                const char* subnetMask,
                const char* hostName,
                std::chrono::seconds leaseTime = std::chrono::seconds(7200),
                std::chrono::seconds offerKeepTime = std::chrono::seconds(60));
        void exec_listen();
        ~Session();
    };
} // namespace bric::Networking::DHCP
