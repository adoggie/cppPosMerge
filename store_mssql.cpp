
#include "store_mssql.h"
// #include "db_mssql.h"

#include <iostream>
#include <cstdlib>
#include <stdlib.h>
//#include <sql.h>
//#include <sqlext.h>
#include <QtCore>

// https://www.sqlapi.com/
// https://www.sqlapi.com/HowTo/

// https://www.easysoft.com/developer/languages/c/odbc_tutorial.html
#include <iostream>
#include <string>
#include <sybfront.h>
#include <sybdb.h>
#include "./utils/logger.h"


void endDatabase(DBPROCESS * dbproc) {
    dbclose(dbproc);
    dbexit();
}

DBPROCESS * prepareDatabase( const StoreMsSql::Settings & config) {
    LOGINREC *login;
    DBPROCESS *dbproc;
    RETCODE result_code;
    std::string server = config.server_addr;
    std::string user = config.db_user;
    std::string password = config.db_password;
    std::string database = config.db_name;
    std::string query = "SELECT count(*) FROM p_follow";

    // Initialize FreeTDS
    if (dbinit() == FAIL) {
        LOG4CPLUS_DEBUG(getLogger("error"), "Failed to initialize FreeTDS");
        return nullptr;
    }

    // Allocate a login structure
    login = dblogin();

    // Set the login credentials
    DBSETLUSER(login, user.c_str());
    DBSETLPWD(login, password.c_str());

    // Connect to the SQL Server
    dbproc = dbopen(login, server.c_str());

    if (dbproc == NULL) {
        LOG4CPLUS_DEBUG(getLogger("error"), "Failed to connect to SQL Server" );
        dbexit();
        return nullptr;
    }

    // Select the database
    result_code = dbuse(dbproc, database.c_str());

    if (result_code == FAIL) {
        LOG4CPLUS_DEBUG(getLogger("error"),"Failed to select database" );
        dbclose(dbproc);
        dbexit();
        return nullptr;
    }

    return dbproc;
}

bool StoreMsSql::init(const QJsonObject& settings){
    // mssql();
    config_.server_addr = settings.value("server_addr").toString("").toStdString();
    config_.db_port = settings.value("port").toInt(1433);
    config_.db_name = settings.value("db").toString("Future").toStdString();
    config_.db_password = settings.value("passwd").toString("").toStdString();
    config_.db_user = settings.value("user").toString("yuyang").toStdString();
    config_.enable = settings.value("enable").toBool(true);
    return true ;
}

bool StoreMsSql::start() {
    stopped_.store(false);
    // redis_ = std::make_shared<sw::redis::Redis>(config_.server_addr);

//    thread_ = std::thread([this]() {
//        LOG4CPLUS_DEBUG(getLogger(), "StoreMsSql::thread start" );
//        while( !stopped_.load()){
//            Item::Ptr item;
//            if (!queue_.dequeue(item)) {
//                continue;
//            }
//
//        }
//        LOG4CPLUS_DEBUG(getLogger(), "StoreMsSql::thread stopped" );
////        std::cout << "StoreRedis::thread:   stopped" << std::endl;
//    });
    return true;
}

void StoreMsSql::stop() {
    stopped_.store(true);
    thread_.join();
}



void StoreMsSql::loadAcEntries( std::vector<AcEntry::Ptr> & acs){
    auto dbproc = prepareDatabase(config_);
//    std::string SQL_CMD = "select ac,f_ac,ratio,symbol from p_follow";
//    std::string SQL_CMD = "select top 200 ac,f_ac, ratio,pratio,stock,type,f_acisbasicacname from p_follow";
    std::string SQL_CMD = "select  ac,f_ac, ratio,pratio,stock,type,f_acisbasicacname from p_follow";

    RETCODE rc;
    rc = dbcmd(dbproc, SQL_CMD.c_str());
    if( rc == FAIL){
        LOG4CPLUS_DEBUG(getLogger("error"), "dbcmd() failed.");
        return;
    }
    if (dbsqlexec(dbproc) == FAIL) {
        LOG4CPLUS_DEBUG(getLogger("error"),"dbsqlexec() failed.\n");
        return;
    }
    
    while ((rc = dbresults(dbproc)) != NO_MORE_RESULTS) {
        if (rc == FAIL) {
            LOG4CPLUS_DEBUG(getLogger("error"),"dbresults failed\n");
            return;
        }
        while (dbnextrow(dbproc) != NO_MORE_ROWS) {
            AcEntry::Ptr ac = std::make_shared<AcEntry>();
            DBINT len;
//            len = dbcollen(dbproc, 1);
//            len = dbcollen(dbproc, 3); // ratio 8
//            len = dbcollen(dbproc, 5); // stock
            ac->ac = std::string( (char*)dbdata(dbproc,1));
            ac->fac = std::string( (char*)dbdata(dbproc,2));
            ac->ratio = *(DBFLT8 *) dbdata(dbproc,3);
            std::string sval = (char*)dbdata(dbproc,5);
            ac->symbol = (SymbolIndex) std::stoul(sval);
            acs.push_back(ac);

        }
    }
    endDatabase(dbproc);
    
}


