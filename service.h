#ifndef _POS_MERGE_MANAGER_H
#define _POS_MERGE_MANAGER_H

#include <string>
#include <thread>
#include <atomic>
// #include <pair>
#include <map>
#include <mutex>
#include <condition_variable>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>



#include "base.h"
#include "worker.h"
#include "feed.h"
#include "fanout.h"
#include "utils/timer.h"
#include "message.h"
#include "store_redis.h"
#include "store_mssql.h"


class Manager: public IFeedUser {
public:
    struct Settings{
        uint32_t fanout_interval;
        uint32_t workers;
        uint32_t sythesize_repeat;
        std::string feed_type;
        std::string position_cache_redis_addr;
    };
    bool init(const std::string& configfile);
    bool start();
    void stop();
    
    static Manager& instance(){
        static Manager s;
        return s;
    }

    void waitForShutdown();
    // log4cplus::Logger& getLogger()  { return logger_;}

protected:
    bool  initTables();

friend class Worker;
friend class Zmq_Feed;
    void onPosMessage(PosMessage * message);
    void onTimer();

    SymbolIndex getSymbolIndex(const std::string& symbol);
    AcIndex getAcIndex(const std::string& ac);
    void addAcEntry(const AcEntry::Ptr ac);
    void initPosition();
    void sythesizePosition(SymbolIndex sidx, uint32_t repeat);
    void onFeedRawData(lob_data_t * data);
    void onMessage(Message * msg);

    void loadPositions(std::vector<PosRecord>& pos_list);
    void loadPosFromRedis(std::vector<PosRecord>& pos_list);
    void loadAcEntries();
private:
   
    std::vector<Worker::Ptr> workers_;

   
    std::thread   thread_;
    std::atomic<bool> stopped_;

    // LockQueue<symbol_px_record_t> queue_;
    std::mutex  mutex_;
    std::condition_variable cond_;
    utils::Timer timer_;
    Settings config_;

    std::map<std::string , SymbolIndex> symbol_index_;
    std::map< SymbolIndex,std::string > index_symbol_;
    std::map<std::string , AcIndex> ac_index_;
    std::map< AcIndex,std::string > index_ac_;
    std::vector<AcIndex>  ac_index_list_;

    std::vector< AcEntry::Ptr> ac_list_;
    std::map<std::string , AcEntry::Ptr> ac_map_;

 
    SymbolPosTable      symbol_pos_table_;
    SymbolRatioTableSet symbol_ratio_tableset_;

    FeedBase::Ptr feeder_;  
    std::vector<FanoutBase::Ptr> fanouts_;
    IFeedDataDecoder::Ptr  decoder_;

    StoreRedis  store_redis_;
    StoreMsSql  store_mssql_;
};


#endif