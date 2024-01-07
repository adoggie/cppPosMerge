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


class Manager {
public:
    struct Settings{
        uint32_t fanout_interval;
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
    
    void onPosMessage(PosMessage * message);
    void onTimer();

    SymbolIndex getSymbolIndex(const std::string& symbol);
    AcIndex getAcIndex(const std::string& ac);
    void addAcEntry(const AcEntry::Ptr ac);
    void initPosition();
    void sythesizePosition(SymbolIndex sidx, uint32_t repeat);
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

};


#endif