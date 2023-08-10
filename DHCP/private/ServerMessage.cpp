#include "../ServerMessage.h"
#include "ServerMessageData.h"
#include "MessageData.h"

using namespace bric::Networking::DHCP;


ServerMessage::ServerMessage() : Message(),d(nullptr) {}

ServerMessage::ServerMessage(const Message& request) : Message(request),d(new Data)
{
    uint8_t* data = reinterpret_cast<uint8_t*>(Message::d->msg);
    d->payload = std::vector<uint8_t>(data,data + sizeof(Header));

    Message::d->msg = (msgdef*)d->payload.data();
    Message::d->nbData = sizeof(Header);
    Message::d->msgType = MessageType::unknown;
    Message::d->option.clear();
}

ServerMessage::ServerMessage(const ServerMessage& request) :Message(request),d(new Data)
{
    Message::d->msg = (msgdef*)d->payload.data();
}

ServerMessage::ServerMessage(ServerMessage&& request) :Message(request),d(request.d)
{
    request.d = nullptr;
    Message::d->msg = (msgdef*)d->payload.data();
}

ServerMessage::~ServerMessage()
{
    delete d;
}