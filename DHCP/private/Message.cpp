#include "MessageData.h"
#include "../Message.h"
#include "../../Exception/ProtocolAnalysisError.h"


using namespace bric::Networking::DHCP;
namespace Exception = bric::Networking::Exception;

class OptionIterator {
    private:
        uint8_t* optionStart;
        uint8_t* current;
    public:
        OptionIterator(uint8_t* start,uint8_t* current) 
            :optionStart(start),current(current){}

        OptionIterator& operator++() {
            Option* current = reinterpret_cast<Option*>(this->current);
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

        Option& operator*(){
            return *reinterpret_cast<Option*>(this->current);
        } 
};

class Options {
    private:
        uint8_t* option;
        size_t length;
    public:
        Options(uint8_t* option, size_t length):option(option),length(length) {}

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

Message::Message(const Buffer& buffer):Message()
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

Message::Message(Message&& msg) noexcept:d(msg.d){
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

    for(Option& i:Options(d->msg->options,d->OptionDataSize()))
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

void Message::loadData(const Buffer& buffer) 
{
    this->loadData(buffer.data,buffer.size);
}

const std::map<OptionType,std::vector<uint8_t>>& Message::options()
{
    return d->option;
}

MessageType Message::messageType()
{
    return d->msgType;
}

size_t MessageData::OptionDataSize()
{
    return nbData - sizeof(Header);
}
