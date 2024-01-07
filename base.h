
#ifndef _POSMERGE_BASE_H
#define _POSMERGE_BASE_H
#include <cstddef>
#include <stdint.h>
#include <cstdlib>
#include <list>
#include <map>
#include <mutex>
#include <atomic>
#include <tuple>
#include <condition_variable>
#include <thread>
#include <algorithm>
#include <sstream>
#include <memory>
#include <vector>
#include <shared_mutex>
#include <string>
#include <cstddef>

typedef std::shared_mutex RwMutex;
typedef void* security_body_addr_t;

#define MAX_ORDER_DEPTH 100  //

typedef uint32_t  symbolid_t;
typedef uint32_t  lob_price_t;
typedef int32_t  lob_qty_t;
typedef std::vector<uint8_t> Buffer;


typedef uint32_t  SymbolIndex;
typedef uint32_t  AcIndex;

const SymbolIndex SYMBOL_INDEX_ALL = 0 ;

template <typename T , typename M= std::mutex>
struct Mutex{
    Mutex():mtx_(){}
    Mutex(const Mutex& ) = delete;
    Mutex& operator=(const Mutex& ) = delete;
    M mtx_;
    T   data_;
    M & getMutex(){return mtx_;}
    T & getData(){return data_;}    
};


// 接收实时策略仓位信号
typedef std::vector<double> AcPosArray;

struct SymbolPosTable{    
    // explicit SymbolPosTable(uint32_t sym_num);
    std::vector< Mutex<AcPosArray , std::shared_mutex > >  positions;
    AcPosArray& getPosArray(SymbolIndex idx);
};

typedef SymbolPosTable SymbolPosResult ; 

typedef std::vector<double> AcRatioArray;
struct RatioTableConfig{
    std::vector< AcRatioArray > ratios;    
    SymbolPosResult calculate(const SymbolPosTable& spt, uint32_t repeat=5);
};


struct SymbolRatioTableSet{
    std::vector<RatioTableConfig> tables;
    RatioTableConfig& getTable(SymbolIndex idx);
};

struct Symbol{
    std::string name ;
    SymbolIndex id;
};

struct lob_data_t{
    char * data ;
    size_t size;
};

struct AcEntry{
    typedef std::shared_ptr< AcEntry> Ptr;
    std::string ac;
    std::string fac;
    double  ratio;
    SymbolIndex symbol;

    std::string uid(){
        std::stringstream ss;
        ss << ac << "_" << fac << "_" << symbol;
        return ss.str();
    }
};


lob_data_t* lob_data_alloc(size_t size);
lob_data_t* lob_data_alloc2(char *data, size_t len);

void lob_data_free(lob_data_t* data);


#endif


