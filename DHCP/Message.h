#pragma once
#include <stdint.h>
#include <stddef.h>

#include <vector>
#include <map>
#include <boost/asio.hpp>


namespace bric::Networking::DHCP {
	class MessageData;

	static uint32_t magicCookie = 0x63538263;
	using streambuf = boost::asio::streambuf;

	enum class opType : uint8_t {
		Request = 1,
		Reply = 2
	};

	enum class MessageType : uint8_t {
		unknown = 0,
		discover = 1,
		offer = 2,
		request = 3,
		decline = 4,
		ack = 5,
		nak = 6,
		release = 7,
		inform = 8,
	};

	enum class OptionType : uint8_t {
		pad = 0,
		subnetMask = 1,
		gateway = 3,
		domainNameServer = 6,
		hostName = 12,
		domainName = 15,
		performRouterDiscover = 31,
		staticRoute = 33,
		vendorSpecificInformation = 43,
		netBiosOverTcpIpNameServer = 44,
		netBiosOverTcpIpNodeType = 46,
		netBiosOverTcpIpScope = 47,
		requestedIpAddress = 50,
		ipAddressLeaseTime = 51,
		dhcpMessageType = 53,
		serverIdentifier = 54,
		parameterRequestList = 55,
		vendorClassIdentifier = 60,
		clientIdentifier = 61,
		domainSearch = 119,
		classlessStaticRoute = 121,
		proxyAutoDiscovery = 252,
		end = 255,
	};

	struct Header {
		opType op;
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
		uint32_t magicCookie;
	};

	class Message :public streambuf
	{
	    private:
			MessageType msgType;
			std::map<OptionType,std::vector<uint8_t>> option;

	    public:
	        Message() noexcept;

			void analysis();

			MessageType messageType();
			const std::map<OptionType,std::vector<uint8_t>>& options();

	};
} // namespace bric::Networking::DHCP
