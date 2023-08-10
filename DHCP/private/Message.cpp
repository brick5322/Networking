#include <string>
#include "MessageData.h"
#include "../Message.h"
#include "../ServerMessage.h"

#include "../../Exception/ProtocolAnalysisError.h"
#include "../../Exception/OptionNotFoundError.h"


using namespace bric::Networking::DHCP;
using namespace boost::asio;

namespace Exception = bric::Networking::Exception;

class OptionIterator {
    private:
        const uint8_t* optionStart;
        const uint8_t* current;
    public:
        OptionIterator(const uint8_t* start,const uint8_t* current) 
            :optionStart(start),current(current){}

        OptionIterator& operator++() {
            const Option* current = reinterpret_cast<const Option*>(this->current);
            switch (current->type)
            {
            case OptionType::pad:
                this->current++;
                break;
            case OptionType::end:
                this->current = nullptr;
                break;
            default:
                this->current += sizeof(Option) + current->length;
                break;
            }
            return *this;
        }

        bool operator!=(const OptionIterator& val) {
            return current == val.current;
        }

        const Option& operator*(){
            return *reinterpret_cast<const Option*>(this->current);
        } 
};

class Options {
    private:
        const uint8_t* option;
        size_t length;
    public:
        Options(const uint8_t* option, size_t length):option(option),length(length) {}

        inline OptionIterator begin() {
            return OptionIterator(option,option);
        }

        inline OptionIterator end() {
            return OptionIterator(option,nullptr);
        }
};


Message::Message() noexcept:d(nullptr) {}

Message::Message(const uint8_t* data,size_t len):Message()
{
    this->loadData(data,len);
}

Message::Message(const_buffer& buffer):Message()
{
    this->loadData(buffer);
}

Message::Message(const Message& msg) :Message()
{
    this->d = new Data;
    d->msg = msg.d->msg;
    d->msgType = msg.d->msgType;
    d->option = msg.d->option;
}

Message::Message(Message&& msg) noexcept:d(msg.d)
{
    msg.d = nullptr;
}

Message::~Message()
{
    delete d;
}

Message::operator bool()
{
    return d;
}

const uint8_t* Message::operator[](OptionType type) 
{
    if(d->option.count(type))
        return d->option[type].data();
    else
        throw Exception::OptionNotFoundError("cannot found messageType:" + std::to_string((uint32_t)type));
}

const Header* Message::operator->()
{
    return d->msg;
}

void Message::loadData(const uint8_t* data,size_t len)
{
    if(!d)
        d = new MessageData;

    d->msg = (msgdef*)data;
    d->nbData = len;

    if (sizeof(Header) > len)
        throw Exception::ProtocolAnalysisError("DHCP - too few bytes accepted");
    
    if (d->msg->op != opType::Request)
        throw Exception::ProtocolAnalysisError("DHCP - invalid message (initial check failed : opcode)");

    if (d->msg->magicCookie != magicCookie)
        throw Exception::ProtocolAnalysisError("DHCP - invalid message (initial check failed : magic_cookie)");
    
    d->option[OptionType::clientIdentifier] = std::vector<uint8_t>(d->msg->chaddr,d->msg->chaddr + sizeof(d->msg->chaddr));

    for(const Option& i:Options(d->msg->options,d->OptionDataSize()))
    {
        if(i.type == OptionType::dhcpMessageType)
            d->msgType = *(MessageType*)i.data;
        else
            d->option[i.type] = std::vector<uint8_t>(i.data,i.data + i.length);
    }

    if(d->msgType == MessageType::unknown)
        throw Exception::ProtocolAnalysisError("DHCP - invalid message (type unknown)");

    if(!d->option.count(OptionType::hostName))
        throw Exception::ProtocolAnalysisError("DHCP - unknown host name");
}

void Message::loadData(const_buffer& buffer) 
{
    this->loadData((uint8_t*)buffer.data(),buffer.size());
}


MessageType Message::messageType()
{
    return d->msgType;
}

size_t MessageData::OptionDataSize()
{
    return nbData - sizeof(Header);
}
