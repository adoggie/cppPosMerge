#include <QCoreApplication>
#include <QByteArray>
#include <QtCore>
#include <qlogging.h>

#include <iostream>
#include <qnumeric.h>

#include <vector>
#include <ranges>
#include <thread>
#include <signal.h>
#include <functional>
#include <cstdlib>
#include "utils/logger.h"
#include "service.h"

bool isEven(int num) {
    return num % 2 == 0;
}

void signalHandler(int signal) {
    if (signal == SIGINT) {        
        Manager::instance().stop();
    }
}

log4cplus::Logger& getLogger(const std::string& name){
    static std::shared_ptr< std::map<std::string,log4cplus::Logger> > loggers;
    if(!loggers){
        loggers = std::make_shared<std::map<std::string,log4cplus::Logger>>();
        (*loggers)["main"] = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("cpposmerge"));
        (*loggers)["error"] = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("error"));
    }

    if( name == "error"){
        return (*loggers)["error"];
    }
    return (*loggers)["main"];
}

int main(int argc, char *argv[]){
   const  char *env = std::getenv("CPPOSMGR_CONFIG");
    if( env == nullptr){
        std::cout<<"CPPOSMGR_CONFIG not set."<<std::endl;
        env = "settings.json";
        // return 1;
    }

    log4cplus::Initializer initializer;
    log4cplus::PropertyConfigurator::doConfigure("cpposmerge.properties");


    LOG4CPLUS_INFO(getLogger(), LOG4CPLUS_TEXT("AppService[cpposmerge] started.."));

    if( !Manager::instance().init(env)){
        LOG4CPLUS_ERROR_FMT(getLogger(),"cpposmerge Service init Error.");
        return 1;
    }
    Manager::instance().start();
    Manager::instance().waitForShutdown();
}

// LOBSTER_CONFIG=/opt/lobster/settings.json ./lobster
//