/***
 * @file feed_zmq.h
 * @brief 接收来自zmq的通联数据规格的L2行情数据
 * @author scott
*/

#ifndef _FEED_ZMQ_H
#define _FEED_ZMQ_H

 #include "base.h"
 #include "message.h"
#include <atomic>
#include <thread>
#include <QJsonObject>
#include "feed.h"

class Zmq_Feed : public FeedBase{
public:
    struct DataDecoder:IFeedDataDecoder{
        Message* decode(lob_data_t* data);
    };

    struct Settings{
        std::string server_addr;
        std::string mode; // bind / connect
    };
public:
    bool init(const QJsonObject& settings) ; 
    ~Zmq_Feed() = default;
    Zmq_Feed() = default;
    bool start();
    void stop();
protected:

private:
    std::atomic<bool> stopped_;
    Settings config_;
    std::thread  thread_;
};


#endif // 