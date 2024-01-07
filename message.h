
#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <stdint.h>
#include <memory>
#include <vector>

#include "base.h"


#pragma pack(push, 1)
struct PosMessage{
    uint32_t ver;
    std::string    topic;
    std::string    symbol;
    std::string    ac;
    double  position;
};
#pragma pack(pop)

struct PosRecord{
    typedef std::shared_ptr<PosRecord> Ptr;
    std::string    symbol;
    std::string    ac;
    double  position;
};

std::shared_ptr<PosMessage> decode_message(const uint8_t* data, size_t len);
void encode_message(const PosMessage& msg, Buffer& buf);

#endif // 
