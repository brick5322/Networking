#include <string>
#include <stdint.h>
#include <stddef.h>

namespace bric::Networking
{
    struct Buffer {
        uint8_t* data;
        size_t size;

        Buffer(uint8_t* data,size_t size):data(data),size(size){}

        operator std::string(){
            return std::string((char*)data,size);
        }
    };
}
