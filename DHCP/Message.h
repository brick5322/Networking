#pragma once
#include <stdint.h>
#include <stddef.h>

#include <vector>
#include <map>
#include <boost/asio.hpp>


namespace bric::Networking::DHCP {
	class MessageData;
	namespace asio = boost::asio;

	constexpr static uint32_t magicCookie = 0x63538263;
	constexpr static uint32_t bufferSize = 0xffff;
	using basic_vector = std::vector<uint8_t>;
    using protocol = asio::ip::udp;

	enum class opType : uint8_t 
	{
		Request = 1,
		Reply = 2
	};

	enum class MessageType : uint8_t 
	{
		Unknown = 0,
		Discover = 1,
		Offer = 2,
		Request = 3,
		Decline = 4,
		Ack = 5,
		Nak = 6,
		Release = 7,
		Inform = 8,
	};

	enum class OptionType : uint8_t 
	{
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
		messageType = 53,
		serverIdentifier = 54,
		parameterRequestList = 55,
		vendorClassIdentifier = 60,
		clientIdentifier = 61,
		domainSearch = 119,
		classlessStaticRoute = 121,
		proxyAutoDiscovery = 252,
		end = 255,
	};

	struct Header 
	{
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

	class Message :public basic_vector
	{
	    private:
			MessageType msgType;
			std::map<OptionType, basic_vector> option;

	    public:
	        Message() noexcept;

			basic_vector& operator[](OptionType);
			const basic_vector& operator[](OptionType) const;
			Header* operator->();

			protocol::endpoint fetch_from(protocol::socket& socket);
			void push_to(protocol::socket& socket,const protocol::endpoint& destination);

			asio::const_buffer packMessage();
			void cleanOptions();
			void setOption(OptionType type,basic_vector&& vec);
			template <typename T>
			void setOption(OptionType type,const T& num) {
			    using tmpType = struct 
			    {
			        uint8_t byte[sizeof(T)/sizeof(uint8_t)];
			    };
			    const tmpType& tmp = reinterpret_cast<const tmpType&>(num);
			    option[type] = basic_vector(tmp.byte,tmp.byte + sizeof(T)/sizeof(uint8_t));
			}

			MessageType messageType();

	};
} // namespace bric::Networking::DHCP
