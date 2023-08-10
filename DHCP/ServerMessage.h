#pragma once
#include <stdint.h>
#include <stddef.h>

#include <vector>
#include <map>

#include "Message.h"

namespace bric::Networking::DHCP
{
    class ServerMessageData;
    class ServerMessage : public Message
    {
        using Data = ServerMessageData;
        private:
            Data *d;
        public:
            ServerMessage() noexcept;
            ServerMessage(const Message& request);
            ServerMessage(const ServerMessage& request);
            ServerMessage(ServerMessage&& request);

            ~ServerMessage();

    };
} // namespace bric::Networking::DHCP
