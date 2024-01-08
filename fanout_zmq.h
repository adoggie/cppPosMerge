#ifndef _LOG_FANOUT_ZMQ_H
#define _LOG_FANOUT_ZMQ_H

#include <string>
#include <vector>
#include <memory>
#include <mutex>

#include <atomic>
#include <thread>
#include <QJsonObject>

#include "base.h"
#include "lockqueue.h"
#include "fanout.h"

class  FanoutZMQ:public FanoutBase{
public:
    typedef std::shared_ptr<FanoutZMQ> Ptr;
    struct Settings{
        std::string server_addr; // zmq fanout server address
        std::string mode;       // bind or connect
        bool enable;
    };

    bool init(const QJsonObject& settings) ;    
    FanoutZMQ() = default;
    ~FanoutZMQ() = default;
    bool start();
    void stop();

private:
    FanoutZMQ::Settings  config_;
};

#endif