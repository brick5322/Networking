#pragma once
#include <exception>
#include <string>

namespace bric::Networking::Exception
{
    class MessageTypeError :public std::exception
    {
        private:
            std::string description;

        public:
            MessageTypeError(const std::string& description)
                :description(std::string("MessageTypeError : " + description + ".\n")) {}

            virtual const char* what()const noexcept override {
                return description.c_str();
            }
    };
} // namespace bric::Networking::Exception
