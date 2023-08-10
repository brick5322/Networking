#include "../Session.h"
#include "../Exception.h"
#include "Message.h"

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
    protocol::socket listener(*this);
    bool isBroadCast = true;
    std::vector<uint8_t> buffer(65536);
    listener.bind(protocol::endpoint(ip::address_v4::any(),serverPort));

    if(setsockopt(listener.native_handle(),SOL_SOCKET,SO_BROADCAST,&isBroadCast,sizeof(isBroadCast)))
        throw boost::system::system_error(asio::error::fd_set_failure);
    
    while (true)
    {
        protocol::endpoint remote;
        size_t length = listener.receive_from(asio::buffer(buffer.data(),65536),remote);
        Message msg(buffer.data(),length);

        try {
            if (this->hostName == (const char*)msg[OptionType::hostName])
                continue;
        } catch (Exception::OptionNotFoundError e) {}

        switch (msg.messageType())
        {
        case MessageType::discover:
            break;
        case MessageType::request:
            break;
        case MessageType::decline:
            break;
        case MessageType::offer:
        case MessageType::ack:
        case MessageType::nak:
            continue;
        case MessageType::release:
            break;
        case MessageType::inform:
            break;
        default:
            break;
        }
    }
    
}