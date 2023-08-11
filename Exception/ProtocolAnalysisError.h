#pragma once
#include <exception>
#include <string>

namespace bric::Networking::Exception
{
    class ProtocolAnalysisError :public std::exception
    {
        private:
            std::string description;

        public:
            ProtocolAnalysisError(const std::string& description)
                :description(std::string("ProtocolAnalysisError : " + description + ".\n")) {}

            virtual const char* what()const noexcept override {
                return description.c_str();
            }
    };
} // namespace bric::Networking::Exception
