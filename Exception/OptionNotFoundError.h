#pragma once
#include <exception>
#include <string>

namespace bric::Networking::Exception
{
    class OptionNotFoundError :public std::exception
    {
        private:
            std::string description;

        public:
            OptionNotFoundError(const std::string& description)
                :description(std::string("OptionNotFoundError : " + description + ".\n")) {}

            const char* what()const noexcept override {
                return description.c_str();
            }
    };
} // namespace bric::Networking::Exception
