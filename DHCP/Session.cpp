#include "Exception.h"
#include "Message.h"
#include "Session.h"

using namespace bric::Networking::DHCP;

Session::Session(const char* hostAddr,const char* dnsAddr,const char* gatewayAddr,const char* minAddr,const char* maxAddr)
    :hostAddr(ip::make_address(hostAddr)),
    dnsAddr(ip::make_address(dnsAddr)),
    gatewayAddr(ip::make_address(gatewayAddr)),
    minAddr(ip::make_address(minAddr)),
    maxAddr(ip::make_address(maxAddr))
{
    protocol::resolver rsv(*this);
    for (auto& result: rsv.resolve(protocol::endpoint(this->hostAddr,0)))
        this->hostName = result.host_name();
}

void Session::exec_listen()
{
    Message msg;
    protocol::socket listener(*this);
    char isBroadCast = true;

    listener.bind(protocol::endpoint(ip::address_v4::any(),serverPort));

    if(setsockopt(listener.native_handle(),SOL_SOCKET,SO_BROADCAST,&isBroadCast,sizeof(isBroadCast)))
        throw boost::system::system_error(asio::error::fd_set_failure);
    while (true)
    {
        protocol::endpoint remote = msg.fetch_from(listener);

        try {
            if (this->hostName == (const char*)msg[OptionType::hostName])
                continue;
        } catch (Exception::OptionNotFoundError& e) {}

        switch (msg.messageType())
        {
        case MessageType::Discover:
            break;
        case MessageType::Request:
            break;
        case MessageType::Decline:
            break;
        case MessageType::Offer:
        case MessageType::Ack:
        case MessageType::Nak:
            continue;
        case MessageType::Release:
            break;
        case MessageType::Inform:
            break;
        default:
            break;
        }
    }
    
}