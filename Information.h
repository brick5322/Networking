#pragma once
#include <string>

namespace bric::Networking
{

    class Information
    {
    private:
        uint32_t gateway;
        uint32_t ipAddress;
        uint32_t subnetMask;
        uint64_t macAddress;
        std::string hostName;

    public:
        Information(const char* gateway,const char* subnetMask);
        Information(const char* gateway,int subnetMask);
        Information(const char* ipAddress);
        ~Information();
    };
} // namespace bric::Networking::ip
