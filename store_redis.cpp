

#include <boost/algorithm/string/case_conv.hpp>
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>


#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "store_redis.h"
#include "utils/strutils.h"

//https://github.com/sewenew/redis-plus-plus

bool StoreRedis::init(const QJsonObject& settings){
    config_.server_addr = settings.value("server_addr").toString("tcp://127.0.0.1:6379").toStdString();

    return true ;
}

bool StoreRedis::start() {
    stopped_.store(false);
    redis_ = std::make_shared<sw::redis::Redis>(config_.server_addr);

    thread_ = std::thread([this]() {
        std::cout << "StoreRedis::thread start" << std::endl;

        while( !stopped_.load()){
            Item::Ptr item;
            if (!queue_.dequeue(item)) {
                continue;
            }
            std::shared_ptr<std::vector<PosRecord>>& prs = item->prs;
            for(auto & pr : *prs){
                std::stringstream ss; 
                ss << pr.symbol << "." << pr.ac ; 
                std::string key = ss.str();                
                std::string value = std::to_string(pr.position);
                redis_->set(key, value);
            }
            
        }
        std::cout << "StoreRedis::thread:   stopped" << std::endl;
    });
    return true;
}

void StoreRedis::stop() {
    stopped_.store(true);
    thread_.join();
}

void StoreRedis::loadPositions(std::vector<PosRecord>& pos_list){
    // load from redis into pos_list
     // Send the KEYS command with the pattern "a.*"
    try {
        std::string pattern = "*";
        std::vector<std::string> keys;     
           
        // auto cursor = redis_->scan(pattern, keys);
        // std::vector<std::string> keys;
        redis_->keys("*", std::back_inserter(keys));

        for (const auto& key : keys) {
            std::string value;
            // redis_->get(key);
            PosRecord pr ;
            auto ss = utils::splitString(key, '.');
            if( ss.size() == 2){
                pr.symbol = ss[0];
                pr.ac = ss[1];
                pr.position = std::stod(value);
                pos_list.emplace_back(pr);
            }
        }
    } catch (const sw::redis::Error& e) {
        std::cerr << "Redis Error: " << e.what() << std::endl;
    }

}

void StoreRedis::savePositions(std::shared_ptr<std::vector<PosRecord>>& pos_list){
    Item::Ptr item = std::make_shared<Item>();
    item->prs = pos_list;
    queue_.enqueue(item);
}