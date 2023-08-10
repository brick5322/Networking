#pragma once
#include "../Message.h"
#include "../ServerMessage.h"

namespace bric::Networking::DHCP 
{
    struct ServerMessageData 
    {
        public:
            std::vector<uint8_t> payload;
        public:
    };
    
}
