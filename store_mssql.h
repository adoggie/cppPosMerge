
#ifndef _DB_MSSQL_H 
#define _DB_MSSQL_H


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

class  StoreMsSql{
public:
    typedef std::shared_ptr<StoreMsSql> Ptr;
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
    void loadAcEntries( std::vector<AcEntry::Ptr> & acs);
    
private:
    StoreMsSql::Settings  config_;
    
    std::atomic<bool>       stopped_;    
    std::thread             thread_;
    LockQueue<Item::Ptr>    queue_;
};
#endif 
