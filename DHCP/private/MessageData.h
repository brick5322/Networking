#pragma once
#include "../Message.h"

namespace bric::Networking::DHCP {

	struct msgdef :public Header 
	{
		uint8_t options[];
	};

	struct Option {
		OptionType type;
		uint8_t length;
		uint8_t data[];
	};

    struct MessageData {
		public:
  	      	msgdef *msg;
  	      	size_t nbData;
			MessageType msgType;
			std::map<OptionType,std::vector<uint8_t>> option;
		public:
			MessageData():msg(nullptr),nbData(0),msgType(MessageType::unknown){}
			size_t OptionDataSize();

    };

}