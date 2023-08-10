#include "../Exception/ProtocolAnalysisError.h"
#include "../Exception/MessageTypeError.h"
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
    protocol::socket listener(*this);
    char isBroadCast = true;
    listener.bind(protocol::endpoint(ip::address_v4::any(),serverPort));

    if(setsockopt(listener.native_handle(),SOL_SOCKET,SO_BROADCAST,&isBroadCast,sizeof(isBroadCast)))
        throw boost::system::system_error(asio::error::fd_set_failure);
    Message msg;
    while (true)
    {
        protocol::endpoint remote;

        msg.resize(0xffff);
        size_t length = listener.receive_from(asio::buffer(msg.data(),msg.size()), remote);
        msg.resize(length);
        msg.analysis();

        if (msg.options().count(OptionType::hostName)){
            std::string msghostName(reinterpret_cast<const char*>(msg.options().at(OptionType::hostName).data()));
            if(msghostName == this->hostName)
                continue;
        }

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