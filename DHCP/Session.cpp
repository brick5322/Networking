#include "Exception.h"
#include "Message.h"
#include "Session.h"

using namespace bric::Networking::DHCP;
using sys_clock = std::chrono::system_clock;

size_t Session::addressIndex(asio::ip::address addr)
{
    uint32_t cur_addr = addr.to_v4().to_uint();
    uint32_t min_addr = minAddr.to_v4().to_uint();
    uint32_t max_addr = maxAddr.to_v4().to_uint();

    if(cur_addr < max_addr || cur_addr > max_addr)
        throw std::overflow_error("address not in pool");
    
    return cur_addr - min_addr;
}

asio::ip::address Session::getAddressAt(int index)
{
    uint32_t cur_addr = minAddr.to_v4().to_uint() + index;
    uint32_t max_addr = maxAddr.to_v4().to_uint();

    if(cur_addr > max_addr)
        throw std::overflow_error("address not in pool");

    return asio::ip::make_address_v4(cur_addr);
}

bool Session::addressAvaliable(uint32_t pos)
{
    using namespace asio::ip;

    address addr = getAddressAt(pos);
    io_context ctx;
    icmp::endpoint dest = *icmp::resolver(ctx).resolve(icmp::v4(),addr.to_string(),"").begin();

    IPInfo& info = this->ipPool[pos];

    // 初始值为1970-1-1 < now
    if (info.releaseTime < sys_clock::now())
    /**
     * @todo 尝试发送icmp (Optional:可以等客户端decline)
    */
        return true;
   return false;
}

Message Session::setReply(const Message& request,const asio::ip::address& offered)
{
    Message ret(request);
    ret->op = opType::Reply;
    ret.cleanOptions();
    ret->hops = 0;
    ret->ciaddr = 0;
    ret->yiaddr = htonl(offered.to_v4().to_uint());
    ret->siaddr = 0;
    memset(ret->file,0,sizeof(ret->file));
    ret.setOption(OptionType::serverIdentifier,htonl(hostAddr.to_v4().to_uint()));
    ret.setOption(OptionType::ipAddressLeaseTime,htonl(leaseTime.count()));
    ret.setOption(OptionType::subnetMask,htonl(subnetMask.to_v4().to_uint()));
    try 
    {
        for(uint8_t param : request[OptionType::parameterRequestList])
        {
            switch ((OptionType&)param)
            {
            case OptionType::subnetMask:
                break;
            case OptionType::gateway:
                ret.setOption(OptionType::gateway,htonl(gatewayAddr.to_v4().to_uint()));
                break;
            case OptionType::domainNameServer:
                ret.setOption(OptionType::domainNameServer,htonl(dnsAddr.to_v4().to_uint()));
                break;
            case OptionType::serverIdentifier:
                break;
            case OptionType::ipAddressLeaseTime:
                break;
            default:
                break;
            }

        }
    }
    catch (Exception::OptionNotFoundError& e) {}
    return ret;
}

Session::Session(const char* hostAddr,
                const char* dnsAddr,
                const char* gatewayAddr,
                const char* minAddr,
                const char* maxAddr,
                const char* subnetMask,
                const char* hostName,
                std::chrono::seconds leaseTime,
                std::chrono::seconds offerKeepTime)
    :hostAddr(asio::ip::make_address(hostAddr)),
    dnsAddr(asio::ip::make_address(dnsAddr)),
    gatewayAddr(asio::ip::make_address(gatewayAddr)),
    minAddr(asio::ip::make_address(minAddr)),
    maxAddr(asio::ip::make_address(maxAddr)),
    subnetMask(asio::ip::make_address(subnetMask)),
    hostName(hostName),
    leaseTime(leaseTime),
    offerKeepTime(offerKeepTime),
    ipPool(inet_addr(maxAddr) - inet_addr(minAddr))
{
}

void Session::exec_listen()
{
    static const uint8_t* unknown_host = (uint8_t*)"unknownClientIdentifier";
    static constexpr int nb_unknown_host = sizeof("unknownClientIdentifier");
    Message request;
    protocol::socket listener(*this);
    char isBroadCast = true;

    listener.bind(protocol::endpoint(asio::ip::address_v4::any(),serverPort));

    if(setsockopt(listener.native_handle(),SOL_SOCKET,SO_BROADCAST,&isBroadCast,sizeof(isBroadCast)))
        throw boost::system::system_error(asio::error::fd_set_failure);

    while (true)
    {
        protocol::endpoint remote = request.fetch_from(listener);

        try {
            if (this->hostName == (const char*)request[OptionType::hostName].data())
                continue;
        } catch (Exception::OptionNotFoundError& e) {}

        switch (request.messageType())
        {
        case MessageType::Discover:
        {

            asio::ip::address retaddr = asio::ip::address_v4::any();
            for(int i = 0;i < ipPool.size();i++)
            {
                if (!this->addressAvaliable(i))
                    continue;
                retaddr = this->getAddressAt(i);
                break;
            }

            if (retaddr == asio::ip::address_v4::any())
            /**
             * @todo Add Warning
            */
                continue; // next listen
            Message reply = this->setReply(request,retaddr);
            reply.setOption(OptionType::messageType,MessageType::Offer);
            reply.push_to(listener);
            break;
        }
        case MessageType::Request:
        {
            try
            {
                uint32_t tmp = *reinterpret_cast<uint32_t*>(request[OptionType::requestedIpAddress].data());
                asio::ip::address_v4 request_ip(ntohl(tmp));
                IPInfo& request_ip_info = ipPool[this->addressIndex(request_ip)];
                request_ip_info.releaseTime = sys_clock::now() + this->leaseTime;
                request_ip_info.clientIdentifier = request[OptionType::requestedIpAddress];
                Message reply = this->setReply(request,request_ip);
                reply.setOption(OptionType::messageType,MessageType::Ack);
                reply.push_to(listener);
                break;
            }
            catch(const Exception::OptionNotFoundError& e)
            {
                Message reply = this->setReply(request,asio::ip::address_v4());
                reply.cleanOptions();
                reply.setOption(OptionType::messageType,MessageType::Nak);
                reply.setOption(OptionType::serverIdentifier,htonl(hostAddr.to_v4().to_uint()));
                reply.push_to(listener);
                break;
            }
            
        }
        case MessageType::Decline:
        {
            try
            {
                uint32_t tmp = *reinterpret_cast<uint32_t*>(request[OptionType::requestedIpAddress].data());
                asio::ip::address_v4 decline_ip(ntohl(tmp));
                IPInfo& decline_ip_info = ipPool[this->addressIndex(decline_ip)];
                decline_ip_info.releaseTime = sys_clock::now();
                decline_ip_info.clientIdentifier = basic_vector(unknown_host,unknown_host + nb_unknown_host);
            }
            catch(const std::exception& e) {}
            break;
        }
        case MessageType::Offer:
        case MessageType::Ack:
        case MessageType::Nak:
            continue;
        case MessageType::Release:
            ipPool[this->addressIndex(remote.address())] = IPInfo();
            break;
        case MessageType::Inform:
            break;
        default:
            break;
        }
    }
    
}

