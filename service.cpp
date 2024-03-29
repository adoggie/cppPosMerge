
#include <algorithm>
#include <fstream>
#include <iostream>
#include <shared_mutex>
#include <tuple>
#include <ctime>
#include <sstream>
#include <functional>
// #include <json/json.h>

#include <QString>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtCore>

#include "service.h"
#include "feed_zmq.h"
#include "fanout_file.h"
#include "fanout_zmq.h"
#include "fanout_redis.h"
#include "utils/logger.h"

bool Manager::init(const std::string& configfile) {
    LOG4CPLUS_INFO(getLogger(), LOG4CPLUS_TEXT("Service::init.."));
    QFile ifs(configfile.c_str());
    // if (!ifs.is_open()) {
    if(!ifs.open(QIODevice::ReadOnly | QIODevice::Text) ){
        QString ss;
        ss =  QCoreApplication::applicationDirPath() + "/settings.json" ;
        // ifs = QFile (ss);
        ifs.setFileName(ss);
        if( !ifs.open(QIODevice::ReadOnly | QIODevice::Text)){
            // std::cerr << "Failed to open config file: " << configfile << std::endl;
            LOG4CPLUS_ERROR(getLogger(), LOG4CPLUS_TEXT("Failed to open config file: ") << configfile);
            return false;
        }
    }

    QByteArray jsonData = ifs.readAll();
    ifs.close();
    
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull()) {
        std::cerr <<  "Failed to parse JSON data" << std::endl;
        return false;
    }

    // Access specific values from the JSON object
    QJsonObject json = jsonDoc.object();
    // config_.lobnum = json.value("lobnum").toInt(1000000);
    config_.feed_type = json.value("feed_type").toString().toStdString();

    auto symbols = json["symbol_list"].toObject();
    for (auto it = symbols.begin();it!= symbols.end();it++){
        std::cout<< it.key().toStdString() << std::string(",") << it.value().toString().toStdString() << std::endl;
    }
    /// load price limit data 
    QDateTime currentDateTime = QDateTime::currentDateTime();
 
    workers_.resize(config_.workers);

    if( config_.feed_type == "feed_zmq"){  // 从zmq 接收
        decoder_ = std::make_shared<Zmq_Feed::DataDecoder>();
        feeder_ = std::make_shared<Zmq_Feed>();
        feeder_->init(json["feed_zmq"].toObject());
    // }else if( config_.feed_type == "feed_redis"){ // 从redis接收实时行情
    //     decoder_ = std::make_shared<Redis_Feed::DataDecoder>();
    //     feeder_ = std::make_shared<Redis_Feed>();
    //     feeder_->init(json["mdl_redis"].toObject());
    }else{
        return false;
    }
    feeder_->setFeedUser(this);
    for (auto& worker : workers_) {
        worker = std::make_shared<Worker>();
        worker->setDataDecoder(decoder_.get());
    }

    store_redis_.init( json["store_redis"].toObject());
    store_mssql_.init( json["store_mssql"].toObject());
    // init fanouts
    auto array = json["fanout"].toArray();
    for ( const auto & obj : array){
        if( obj.isObject() ){
            auto enable = obj.toObject().value("enable").toBool(false);

            if( enable){
               FanoutBase::Ptr fanout;
                if( obj.toObject().value("name") == "zmq"){
                    fanout = std::make_shared< FanoutZMQ >();                    
                }else if( obj.toObject().value("name") == "file"){
                    fanout = std::make_shared< FanoutFile>();
                }else if( obj.toObject().value("name") == "redis"){
                    fanout = std::make_shared<FanoutRedis>();
                }
                if( fanout ) {
                    fanout->init(obj.toObject());
                    fanouts_.push_back(fanout);
                }
            }
        }
    }
    return true;
}

void Manager::addAcEntry(const AcEntry::Ptr ac){
    if( index_symbol_.find(ac->symbol) == index_symbol_.end()){
        return ;        
    }
    auto itr = ac_map_.find(ac->uid());
    if(itr!= ac_map_.end()){
        itr->second->ratio += ac->ratio ;  // 相同的ac，多次配置，累加
        return;
    }
    if( ac->symbol == SYMBOL_INDEX_ALL){
        std::vector<SymbolIndex> syms;
        std::transform(symbol_index_.begin(), symbol_index_.end(), 
                std::back_inserter(syms),[]( auto &s){ return s.second;});
        for( auto & s : syms){
            auto ac2 = std::make_shared<AcEntry>();            
            ac2->symbol = s;
            ac2->ratio = ac->ratio;
            ac2->ac = ac->ac;
            ac2->fac = ac->fac;
            ac_map_.insert( std::make_pair(ac2->uid(), ac2) ) ;
            ac_list_.push_back(ac2);
        }
    }else{
        ac_map_.insert( std::make_pair(ac->uid(), ac) ) ;
        ac_list_.push_back(ac);
    }
    if( ac_index_.find(ac->ac) == ac_index_.end()){        
        ac_index_.insert( std::make_pair(ac->ac, ac_index_list_.size()) );
        index_ac_.insert( std::make_pair(ac_index_list_.size(), ac->ac) );
        ac_index_list_.push_back(ac_index_list_.size());
    }
    if( ac_index_.find(ac->fac) == ac_index_.end()){        
        ac_index_.insert( std::make_pair(ac->fac, ac_index_list_.size()) );
        index_ac_.insert( std::make_pair(ac_index_list_.size(), ac->fac) );
        ac_index_list_.push_back(ac_index_list_.size());
    }
}

// load latest position from database 
void Manager::initPosition () {
    // init symbol_pos_table_
    std::vector<PosRecord> pos_list;
    store_redis_.loadPositions(pos_list);

    // 从数据库读取最新的仓位信息
    for(auto &pr:pos_list){
        try{
            auto sidx = getSymbolIndex(pr.symbol);
            auto & acpmtx = symbol_pos_table_.positions.at(sidx);
            std::unique_lock< std::shared_mutex> lock(acpmtx.getMutex());
            auto& acp = acpmtx.getData();
            auto acidx = getAcIndex(pr.ac);
            auto& p = acp.at(acidx);
            p = pr.position;
        }catch(const std::exception& e){
            LOG4CPLUS_WARN(getLogger("error"), LOG4CPLUS_TEXT("initPosition error:") << e.what());
            LOG4CPLUS_WARN(getLogger("error"), LOG4CPLUS_TEXT("error:") << pr.symbol << " " << pr.ac << " " << pr.position);  
        }
    }
    
}

void Manager::loadAcEntries(){
    std::vector<AcEntry::Ptr> acs;    
    store_mssql_.loadAcEntries(acs);
    for(auto & ac : acs){
        addAcEntry(ac);
    }
}

bool Manager::initTables(){
    std::vector< AcEntry::Ptr> ens;

    std::vector<SymbolIndex> syms;
    std::transform(symbol_index_.begin(), symbol_index_.end(), 
        std::back_inserter(syms),
        []( auto s){ return s.second;});
    SymbolIndex maxIdx = *std::max_element(syms.begin(),syms.end());

    // init symbol_index_ and ac_index_
    // init symbol_pos_table_
    
    symbol_pos_table_.positions = std::vector< Mutex<AcPosArray , std::shared_mutex > >(maxIdx);

    for( auto & acpmtx : symbol_pos_table_.positions){
        acpmtx.getData().resize(ac_index_.size());
        std::fill(acpmtx.getData().begin(), acpmtx.getData().end(), std::numeric_limits<double>::quiet_NaN());
    }
    
    // init ratio_tableset_
    symbol_ratio_tableset_.tables.resize(symbol_index_.size());
    for( auto & ratio_table : symbol_ratio_tableset_.tables){
        ratio_table.ratios.resize(ac_index_.size());
        for( auto & r : ratio_table.ratios){
            r.resize(ac_index_.size());
            std::fill(r.begin(), r.end(), std::numeric_limits<double>::quiet_NaN());
        }
    }

    for( auto & ac : ac_list_){
        auto & ratio_table = symbol_ratio_tableset_.tables.at(ac->symbol);
        auto & ratio_arr = ratio_table.ratios.at(ac_index_.at(ac->ac)) ;
        ratio_arr.at(ac_index_.at(ac->fac)) = ac->ratio;        
    }
    return true;
}

bool Manager::start() {
    store_redis_.start();
    store_mssql_.start();

    for (auto& worker : workers_) {
        worker->start();
    }
    feeder_->start();
    for( auto & fanout : fanouts_){
        fanout->start();
    }
    if( fanouts_.size()){
        timer_.start( config_.fanout_interval, std::bind(&Manager::onTimer,this) );
    }

    stopped_.store(false);
    return true;
}

void Manager::stop() {
    feeder_->stop();
    for (auto& worker : workers_) {
        worker->stop();
    }
    for(auto & fanout : fanouts_){
        fanout->stop();
    }
    stopped_.store(true);
    thread_.join();


    cond_.notify_all();
}


void Manager::waitForShutdown(){
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this]{ return stopped_.load(); });
}

// 更新仓位值 , symbol->ac->position
void Manager::onPosMessage(PosMessage * msg){
    try{
        auto sidx = getSymbolIndex(msg->symbol);
        auto & acpmtx = symbol_pos_table_.positions.at(sidx);
        std::unique_lock< std::shared_mutex> lock(acpmtx.getMutex());
        auto& acp = acpmtx.getData();
        auto acidx = getAcIndex(msg->ac);
        auto& p = acp.at(acidx);
        p = msg->position;
    }catch(const std::exception& e){
        LOG4CPLUS_WARN(getLogger("error"), LOG4CPLUS_TEXT("onPosMessage error:") << e.what());
        LOG4CPLUS_WARN(getLogger("error"), LOG4CPLUS_TEXT("error:") << msg->symbol << " " << msg->ac << " " << msg->position);
    }
}

void Manager::sythesizePosition(SymbolIndex sidx, uint32_t repeat){
    auto & acpmtx = symbol_pos_table_.positions.at(sidx);
    std::shared_lock< std::shared_mutex> lock(acpmtx.getMutex());
    auto& acp = acpmtx.getData(); // AcPosArray
    auto & ratio_table = symbol_ratio_tableset_.tables.at(sidx);
    // auto & ratio_arr = ratio_table.ratios;
    std::vector<double>& a = acp;
    std::vector<double> r(a.size());
    for(auto n =0 ;n< repeat; n++){    
        uint32_t i = 0;    
        for(auto & b : ratio_table.ratios){
            std::vector<double> c,d;
            std::transform(a.begin(), a.end(), b.begin(), std::back_inserter(c), std::multiplies<double>());
            std::copy_if(c.begin(), c.end(), std::back_inserter(d),
                    [](double value) { return !std::isnan(value); });
            auto sum = std::accumulate(d.begin(), d.end(), 0);
            r[i++] = sum;
        }
        a = r;
    }
    std::string sym_name = index_symbol_.at(sidx);
    for(AcIndex n = 0 ;n< r.size() ;n++){
        double p = r[n];
        if(std::isnan(p)){
            continue;
        }
        std::string ac = index_ac_.at(n);
        PosRecord::Ptr pr = std::make_shared<PosRecord>();
        pr->ac = ac;
        pr->position = p;
        pr->symbol = sym_name;
        auto item = std::make_shared<FanoutBase::Item>();
        item->pr = pr;
        for( auto & fanout : fanouts_){
            fanout->fanout(item);
        }
    }
    

}

// sampling & fanout
void Manager::onTimer(){
    // sythesizePosition();
    for(auto& si : symbol_index_){
        sythesizePosition(si.second, config_.sythesize_repeat);
    }
}

void Manager::onFeedRawData(lob_data_t * data){

}

void Manager::onMessage(Message * msg){
    
}


SymbolIndex Manager::getSymbolIndex(const std::string& symbol){
    return symbol_index_.at(symbol);
}

AcIndex Manager::getAcIndex(const std::string& ac){
    return ac_index_.at(ac);
}