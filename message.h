
#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <stdint.h>
#include <memory>
#include <vector>

#include "base.h"

struct Message{

};

#pragma pack(push, 1)
struct PosMessage:Message{
    uint32_t ver;
    std::string    topic; //25
    std::string    symbol; //8
    std::string    ac; //95
    double  position;
    static const uint32_t PACKET_SIZE = 140;
    
    static PosMessage * decode(const uint8_t* data, size_t len);
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
