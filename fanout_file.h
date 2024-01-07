#ifndef _LOG_FANOUT_FILE_H
#define _LOG_FANOUT_FILE_H

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include "lob.h"
#include <atomic>
#include <thread>
#include <QJsonObject>
#include <QFile>
#include "lockqueue.h"

#include "fanout.h"

class  FanoutFile:public FanoutBase{
public:
    typedef std::shared_ptr<FanoutFile> Ptr;
    struct Settings{
        std::string store_dir; // zmq fanout server address        
        bool enable;
    };

    bool init(const QJsonObject& settings) ;
    FanoutFile() = default;
    ~FanoutFile() = default;
    bool start();
    void stop();
    
private:
    FanoutFile::Settings  config_;
    std::map< symbolid_t, std::shared_ptr<QFile> > files_;
};

#endif