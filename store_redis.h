#ifndef _STORE_REDIS_H
#define _STORE_REDIS_H

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include "base.h"
#include <atomic>
#include <thread>
#include <QJsonObject>
#include <QFile>

#include <sw/redis++/redis++.h>

#include "message.h"
#include "lockqueue.h"




class  StoreRedis{
public:
    typedef std::shared_ptr<StoreRedis> Ptr;
    struct Settings{
        std::string server_addr; //  fanout server address        
        bool enable;
    };

    struct Item{
        typedef std::shared_ptr<Item> Ptr;
        PosRecord::Ptr  pr;
        std::shared_ptr<std::vector<PosRecord>> prs;
    };

    bool init(const QJsonObject& settings) ;
    bool start();
    void stop();
    void loadPositions(std::vector<PosRecord>& pos_list);
    void savePositions( std::shared_ptr<std::vector<PosRecord>>& pos_list);
private:
    StoreRedis::Settings  config_;
    std::shared_ptr<sw::redis::Redis> redis_;
    std::atomic<bool>       stopped_;    
    std::thread             thread_;
    LockQueue<Item::Ptr>    queue_;
};

#endif