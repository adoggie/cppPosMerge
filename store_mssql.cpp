
#include "store_mssql.h"
// #include "db_mssql.h"

#include <iostream>
#include <sql.h>
#include <sqlext.h>



bool StoreMsSql::init(const QJsonObject& settings){
    config_.server_addr = settings.value("server_addr").toString("tcp://127.0.0.1:6379").toStdString();

    return true ;
}

bool StoreMsSql::start() {
    stopped_.store(false);
    // redis_ = std::make_shared<sw::redis::Redis>(config_.server_addr);

    thread_ = std::thread([this]() {
        std::cout << "StoreRedis::thread start" << std::endl;

        while( !stopped_.load()){
            Item::Ptr item;
            if (!queue_.dequeue(item)) {
                continue;
            }
           
        }
        std::cout << "StoreRedis::thread:   stopped" << std::endl;
    });
    return true;
}

void StoreMsSql::stop() {
    stopped_.store(true);
    thread_.join();
}



void StoreMsSql::loadAcEntries( std::vector<AcEntry::Ptr> & acs){

}

// https://www.sqlapi.com/
// https://www.sqlapi.com/HowTo/

// https://www.easysoft.com/developer/languages/c/odbc_tutorial.html

int mssql() {
    SQLHENV envHandle; // 环境句柄
    SQLHDBC dbcHandle; // 数据库连接句柄
    SQLHSTMT stmtHandle; // SQL语句句柄
    SQLRETURN retCode;

    // 初始化环境句柄
    retCode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &envHandle);
    retCode = SQLSetEnvAttr(envHandle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_UINTEGER);

    // 初始化数据库连接句柄
    retCode = SQLAllocHandle(SQL_HANDLE_DBC, envHandle, &dbcHandle);

    // 建立数据库连接
    std::string connectionString = "DRIVER={ODBC Driver 17 for SQL Server};SERVER=your_server_address;DATABASE=your_database_name;UID=your_username;PWD=your_password;";
    SQLCHAR* connStr = (SQLCHAR*)connectionString.c_str();
    retCode = SQLDriverConnect(dbcHandle, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

    // 创建SQL语句句柄
    retCode = SQLAllocHandle(SQL_HANDLE_STMT, dbcHandle, &stmtHandle);

    // 执行SQL查询语句
    retCode = SQLExecDirect(stmtHandle, (SQLCHAR*)"SELECT * FROM your_table_name", SQL_NTS);

    // 处理结果集
    SQLCHAR col1Data[255], col2Data[255];
    SQLLEN indicator1, indicator2;

    while (retCode = SQLFetch(stmtHandle) != SQL_NO_DATA) {
        retCode = SQLGetData(stmtHandle, 1, SQL_C_CHAR, col1Data, sizeof(col1Data), &indicator1);
        retCode = SQLGetData(stmtHandle, 2, SQL_C_CHAR, col2Data, sizeof(col2Data), &indicator2);

        // 处理从表中读取的数据
        std::cout << "Column 1: " << std::string((char*)col1Data) << std::endl;
        std::cout << "Column 2: " << std::string((char*)col2Data) << std::endl;
    }

    // 释放资源
    retCode = SQLFreeHandle(SQL_HANDLE_STMT, stmtHandle);
    retCode = SQLDisconnect(dbcHandle);
    retCode = SQLFreeHandle(SQL_HANDLE_DBC, dbcHandle);
    retCode = SQLFreeHandle(SQL_HANDLE_ENV, envHandle);

    return 0;
}
