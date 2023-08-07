#include <string>
#include <stdint.h>
#include <stddef.h>

namespace Networking
{
    struct Buffer {
        uint8_t* data;
        size_t size;

        operator std::string();
    }
} 
