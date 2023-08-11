#include "Message.h"
#include "Exception.h"


using namespace bric::Networking::DHCP;
using namespace boost::asio;
namespace Exception = bric::Networking::Exception;

struct Option 
{
		OptionType type;
		uint8_t length;
		uint8_t data[];
};

struct msgdef :public Header 
{
	uint8_t options[];
};

class OptionIterator 
{
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
            return current != val.current;
        }

        const Option& operator*(){
            return *reinterpret_cast<const Option*>(this->current);
        } 
};

class Options 
{
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

Message::Message() noexcept:basic_vector(bufferSize), msgType(MessageType::Unknown) {}

uint8_t* Message::operator[](OptionType type)
{
    try {
        return option.at(type).data();
    } catch (std::out_of_range& e){
        throw Exception::OptionNotFoundError("cannot find option");
    }
}

protocol::endpoint Message::fetch_from(protocol::socket& socket)
{
    protocol::endpoint remote;
    const msgdef * msg = reinterpret_cast<const msgdef*>(this->data());
    size_t len = socket.receive_from(buffer(this->data(),this->size()), remote);

    if (sizeof(Header) > len)
        throw Exception::ProtocolAnalysisError("DHCP - too few bytes accepted");
    
    if (msg->op != opType::Request)
        throw Exception::ProtocolAnalysisError("DHCP - invalid message (initial check failed : opcode)");

    if (msg->magicCookie != magicCookie)
        throw Exception::ProtocolAnalysisError("DHCP - invalid message (initial check failed : magic_cookie)");
    
    option[OptionType::clientIdentifier] = std::vector<uint8_t>(msg->chaddr,msg->chaddr + sizeof(msg->chaddr));

    for(const Option& i:Options(msg->options,len - sizeof(Header)))
    {
        if(i.type == OptionType::dhcpMessageType)
            msgType = *(const MessageType*)i.data;
        else
            option[i.type] = std::vector<uint8_t>(i.data,i.data + i.length);
    }

    if(msgType == MessageType::Unknown)
        throw Exception::ProtocolAnalysisError("DHCP - invalid message (type unknown)");

    if(!option.count(OptionType::hostName))
        throw Exception::ProtocolAnalysisError("DHCP - unknown host name");

    return remote;
}

const std::map<OptionType,std::vector<uint8_t>>& Message::options()
{
    return option;
}

MessageType Message::messageType()
{
    return msgType;
}
