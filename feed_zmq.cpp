#include "feed_zmq.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

#include <QtCore>
#include "utils/logger.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
// #include <czmq.h>
#include <zmq.hpp>

#include "base.h"
#include "strutils.h"
#include "message.h"
#include "service.h"


bool Zmq_Feed::init(const QJsonObject& json) {
    config_.server_addr = json.value("server_addr").toString("tcp://127.0.0.1:6379").toStdString();
    config_.mode = json.value("mode").toString("bind").toStdString();    
    return true;
}


bool Zmq_Feed::start() {
    stopped_.store(false);

    thread_ = std::thread([this]() {
        std::cout << "thread1 start" << std::endl;
        zmq::context_t context(1);
        zmq::socket_t socket(context, ZMQ_SUB);
        if( boost::algorithm::to_lower_copy(config_.mode) == "connect"){
            socket.connect(config_.server_addr.c_str());
        }else{
            socket.bind(config_.server_addr.c_str());
        }
        socket.set(zmq::sockopt::subscribe, "");

        // http://czmq.zeromq.org/
    // https://github.com/zeromq/cppzmq

        zmq::pollitem_t items[] = { {socket, 0, ZMQ_POLLIN, 0} };
        while (stopped_.load() == false) {
            zmq::poll(items, 1, std::chrono::milliseconds(1));

            if (items[0].revents & ZMQ_POLLIN) {
                // Data is available to be received
                zmq::message_t message;
                auto optret = socket.recv(message, zmq::recv_flags::dontwait); // .value_or((size_t)0);
                if (optret.value() <= 0) {
                    continue;
                }
                
                try {                    
                    PosMessage * msg = PosMessage::decode((uint8_t*) message.data() , message.size() );
                    if( msg ){
                        Manager::instance().onPosMessage(msg);
                    }
                }catch (std::exception& e ) {
                    
                    continue;
                }
                
            }
        }
        std::cout << "thread: mdl zmq  stopped" << std::endl;
    });
    return true;
}

void Zmq_Feed::stop() {
    stopped_.store(true);
    thread_.join();
}



Message* Zmq_Feed::DataDecoder::decode(lob_data_t* data)
{
    Message* msg = nullptr;
    try {
        std::string text = std::string(data->data, data->size);
        auto ss = lobutils::splitString(text, ',');
        if(text.size() == 0){
            return nullptr;
        }
        std::string &mdl_id = ss.back();

        LOG4CPLUS_DEBUG(getLogger(), text);

    }catch(std::exception& e){
        qWarning() << QString("Mdl_Zmq_Feed::Decode() Error: %1").arg(e.what());
    }
    return msg;
}

