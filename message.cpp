#include "message.h"
#include <cstring>
#include <string>
#include <arpa/inet.h> // For ntohs and ntohl


PosMessage* PosMessage::decode(const uint8_t* data, size_t len) {
    if (len < PosMessage::PACKET_SIZE) {
        return nullptr; // Invalid data length
    }

    PosMessage* posMsg = new PosMessage;
    memcpy(&posMsg->ver, data, sizeof(posMsg->ver));
    posMsg->ver = ntohl(posMsg->ver);
    uint32_t topicLen;
    const uint8_t* cur = data + sizeof(posMsg->ver);
    posMsg->topic.assign(cur, cur + 25);
    cur += 25;
    posMsg->symbol.assign(cur, cur + 8);
    cur += 8 ;
    posMsg->ac.assign(cur, cur + 95);
    cur += 95;
    
    memcpy(&posMsg->position, cur , sizeof(posMsg->position));
    uint64_t position = be64toh(*reinterpret_cast<const uint64_t*>(&posMsg->position));
    posMsg->position = *reinterpret_cast<const double*>(&position);

    return posMsg;
}