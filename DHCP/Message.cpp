#include "Message.h"
#include "Exception.h"


using namespace bric::Networking::DHCP;
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
    public:
        Options(const uint8_t* option):option(option) {}

        inline OptionIterator begin() {
            return OptionIterator(option,option);
        }

        inline OptionIterator end() {
            return OptionIterator(option,nullptr);
        }
};

Message::Message() noexcept:basic_vector(bufferSize), msgType(MessageType::Unknown) {}

const basic_vector& Message::operator[](OptionType type) const
{
    try 
    {
        return option.at(type);
    }
    catch (std::out_of_range& e)
    {
        throw Exception::OptionNotFoundError("cannot find option");
    }
}

basic_vector& Message::operator[](OptionType type)
{
    try 
    {
        return option.at(type);
    }
    catch (std::out_of_range& e)
    {
        throw Exception::OptionNotFoundError("cannot find option");
    }
}

Header* Message::operator->()
{
    return reinterpret_cast<Header*>(this->data());
}

protocol::endpoint Message::fetch_from(protocol::socket& socket)
{
    protocol::endpoint remote;
    const msgdef * msg = reinterpret_cast<const msgdef*>(this->data());
    size_t len = socket.receive_from(asio::buffer(this->data(),this->size()), remote);

    if (sizeof(Header) > len)
        throw Exception::ProtocolAnalysisError("DHCP - too few bytes accepted");
    
    if (msg->op != opType::Request)
        throw Exception::ProtocolAnalysisError("DHCP - invalid message (initial check failed : opcode)");

    if (msg->magicCookie != magicCookie)
        throw Exception::ProtocolAnalysisError("DHCP - invalid message (initial check failed : magic_cookie)");
    
    option[OptionType::clientIdentifier] = std::vector<uint8_t>(msg->chaddr,msg->chaddr + sizeof(msg->chaddr));

    for(const Option& i:Options(msg->options))
    {
        if(i.type == OptionType::messageType)
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

void Message::push_to(protocol::socket& socket){
    asio::const_buffer buffer = this->packMessage();
    socket.send(buffer);
}

asio::const_buffer Message::packMessage()
{
    msgdef *msg = reinterpret_cast<msgdef*>(this->data());
    uint8_t* cur_pos = msg->options;
    for (auto& pair : this->option)
    {
        Option *cur_field = reinterpret_cast<Option*>(cur_pos);
        size_t size = pair.second.size();
        cur_field->type = pair.first;
        cur_field->length = size;
        memcpy(cur_field->data,pair.second.data(),size);
        cur_pos += sizeof(Option) + size;
    }
    *cur_pos = (uint8_t)OptionType::end;

    return asio::const_buffer(
                    this->data(),
                    sizeof(Header) + (cur_pos - msg->options + 1));
}

void Message::cleanOptions()
{
    option.clear();
}

MessageType Message::messageType()
{
    return msgType;
}


void Message::setOption(OptionType type,basic_vector&& vec)
{
    option[type] = vec;
}

template <typename T>
void Message::setOption(OptionType type,const T& num)
{
    using tmpType = struct 
    {
        uint8_t byte[sizeof(T)/sizeof(uint8_t)];
    };
    const tmpType& tmp = reinterpret_cast<const tmpType&>(num);
    option[type] = basic_vector(&tmp.byte,&tmp.byte + sizeof(T)/sizeof(uint8_t));
}