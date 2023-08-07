#include <stdint.h>
#include <stddef.h>

#include <vector>
#include <map>

#include "../Buffer.h"

namespace bric::Networking::DHCP {
	enum class MessageType {
		discover = 1,
		offer = 2,
		request = 3,
		decline = 4,
		ack = 5,
		nak = 6,
		release = 7,
		inform = 8,
	};

	enum class OptionType {
		pad = 0,
		subnetMask = 1,
		hostName = 12,
		requestedIpAddress = 50,
		ipAddressLeaseTime = 51,
		dhcpMessageType = 53,
		serverIdentifier = 54,
		clientIdentifier = 61,
		end = 255,
	};

	struct fixed_msg 
	{
		uint8_t op;
		uint8_t htype;
		uint8_t hlen;
		uint8_t hops;
		uint32_t xid;
		uint16_t secs;
		uint16_t flags;
		uint32_t ciaddr;
		uint32_t yiaddr;
		uint32_t siaddr;
		uint32_t giaddr;
		uint8_t chaddr[16];
		uint8_t sname[64];
		uint8_t file[128];
		uint8_t magicCookie[4];
	} DHCPMessageDef;

	struct msgdef :public fixed_msg 
	{
		uint8_t options[];
	};

	class Message {
	    private:
	        msgdef *msg;
	        size_t datalen;
			std::map<OptionType,std::vector<uint8_t>> option;

	    public:
	        Message();
	        Message(const uint8_t* data,size_t datalen);
			Message(const Buffer& buffer);
			void loadData(const uint8_t* data,size_t datalen);
			void loadData(const Buffer& buffer);
	};
}	