
#include "fanout.h"

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
// #include <czmq.h>
#include <zmq.hpp>

#include "base.h"

// LobRecordFanout::LobRecordFanout(const LobRecordFanout::Settings& settings)
// {
//     config_ = settings;
// }

bool FanoutBase::init(const QJsonObject& settings){
    return false;
}

bool FanoutBase::start() {
    return false;
}

void FanoutBase::stop() {
    stopped_.store(true);
    thread_.join();
}

void FanoutBase::fanout( Item::Ptr& item){
    queue_.enqueue(item);
}