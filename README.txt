
# cppPosMerge 
商品期货仓位聚合服务

## 流程

 sqlserver -> lastest posFollowRatioList
 strategies -> zmq -> living positon include: "strategy_name,symbol,position" 
 merging:  
    "ratios and living position , output: real-final-position for each account "
 delivery position -> zmq 
          position -> redis -> write db: position-table


## 配置

    symbols: 交易品种 , 'A,m,Px,...'
    ac -> fac :  账号、策略、策略组合id 

## 运行

品种数: N 
ac_index : 每个ac赋值唯一的索引 , ac_idx
symbol_index : 每个symbol 赋值唯一索引 ,symbox_idx

A=0, 0.1
Y=1, 0.5

typedef uint32_t  SymbolIndex;
typedef uint32_t  AcIndex;
接收实时策略仓位信号
typedef std::vector<double> AcPosArray;
struct SymbolPosTable{    
    explicit SymbolPosTable(uint32_t sym_num);

    std::vector< AcPosArray > positions;
    AcPosArray& getPosArray(SymbolIndex idx);
}
typedef SymbolPosTable SymbolPosResult ; 

typedef std::vector<double> AcRatioArray;
struct RatioTableConfig{
    std::vector< AcRatioArray > ratios;    
    SymbolPosResult calculate(const SymbolPosTable& spt, uint32_t repeat=5);
}

struct SymbolRatioTableSet{
    std::vector<RatioTableConfig> tables;
    RatioTableConfig& getTable(SymbolIndex idx);
}


在C++中，浮点数的NaN值可以使用std::numeric_limits<float>::quiet_NaN()（对于float类型）或std::numeric_limits<double>::quiet_NaN()（对于double类型）来获得。这些函数返回一个特殊的NaN值。

对于整数类型，C++标准库中没有直接提供NaN值的表示方式。一种常见的做法是使用特定的常量，如std::numeric_limits<int>::min()或std::numeric_limits<int>::max()，来表示无效的数值。

需要注意的是，NaN值与任何其他值以及自身的比较都会返回false。因此，在使用NaN时，需要使用特定的函数（例如std::isnan()）来检查它是否等于NaN。


symbolPosTable[symbol_index] = [ ac_idx , .. ]  


ratioTable[ac_idx] = 

settings.json 
"cmake.configureSettings": {
        "CMAKE_TOOLCHAIN_FILE": "/home/zhiyuan/vcpkg/scripts/buildsystems/vcpkg.cmake",
    }

cpplint 
https://cloud.tencent.com/developer/article/1494003

## vcpkg install 
vcpkg install log4cplus protobuf cppzmq jsoncpp
vcpkg install redis-plus-plus hiredis boost-lockfree

https://github.com/boostorg/lockfree/tree/develop
https://github.com/DNedic/lockfree/tree/main
https://github.com/sewenew/redis-plus-plus
https://github.com/log4cplus/log4cplus
https://github.com/log4cplus/log4cplus/wiki/Code-Examples

## protobuf compile 

protoc -I ./ --cpp_out ./ lob.proto
https://protobuf.dev/
protobuf:x64-linux                                3.21.12 

https://github.com/protocolbuffers/protobuf/releases/download/v21.0/protoc-21.0-linux-x86_64.zip
protoc --version 
    libprotoc 3.21.0


Tonglian mdl:redis 
    redis-cli -p 59379 -h 172.16.30.12
    subscribe mdl.23.1.*