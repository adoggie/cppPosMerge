#ifndef _LOG_FANOUT_H
#define _LOG_FANOUT_H

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include "lob.h"
#include <atomic>
#include <thread>
#include <QJsonObject>
#include "lockqueue.h"
#include "message.h"

// template<typename T>
// struct FanoutItem{
//     T   item;
// };

class  FanoutBase{
public:
    struct Item{
        typedef std::shared_ptr<Item> Ptr;
        PosRecord::Ptr  pr;
    };

    typedef std::shared_ptr<FanoutBase> Ptr;
    struct Settings{
        std::string server_addr; // zmq fanout server address
        std::string mode;       // bind or connect        
    };
    FanoutBase() = default;
    virtual bool init(const QJsonObject& settings) ;
    ~FanoutBase() = default;
    virtual bool start();
    virtual void stop();

    // template <typename T>
    // void fanout( const FanoutItem<T> & record);
    virtual void fanout( Item::Ptr & item);
    
protected:
    std::atomic<bool>       stopped_;    
    std::thread             thread_;
    LockQueue<Item::Ptr>    queue_;
};

#endif