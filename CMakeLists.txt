cmake_minimum_required(VERSION 3.14)
set(CMAKE_TOOLCHAIN_FILE "/home/zhiyuan/vcpkg/scripts/buildsystems/vcpkg.cmake")
# set(CMAKE_PREFIX_PATH "/home/zhiyuan/vcpkg/packages")
project(cpposmerge LANGUAGES CXX)

add_compile_definitions(USE_VCPKG)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include_directories(/home/zhiyuan/projects/include)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
message(STATUS "Qt${QT_VERSION_MAJOR}::Core")
# find_package(cppzmq)
find_package(jsoncpp  CONFIG REQUIRED)
find_package(cppzmq  CONFIG REQUIRED)
# find_package(protobuf CONFIG REQUIRED)

# if(PROTOBUF_FOUND)
#   message(STATUS "protobuf_FOUND")
#   message(STATUS "protobuf_VERSION ${protobuf_VERSION}")
#   message(STATUS "protobuf_Libs ${protobuf_LIBRARIES}")

# endif()

find_package(log4cplus CONFIG REQUIRED)
find_package(msgpack-cxx CONFIG REQUIRED)



find_package(hiredis CONFIG REQUIRED)
find_package(redis++ CONFIG REQUIRED)

add_executable(cpposmerge
  main.cpp service.cpp feed_zmq.cpp feed_zmq.h  worker.cpp worker.h 
  fanout.cpp fanout.h
  fanout_zmq.h fanout_zmq.cpp fanout_file.h fanout_file.cpp fanout_redis.cpp fanout_redis.h
  message.h message.cpp store_redis.cpp store_redis.h
  store_mssql.cpp store_mssql.h
  # proto/lob.pb.cc
  utils/logger.h)

target_link_libraries(cpposmerge Qt${QT_VERSION_MAJOR}::Core JsonCpp::JsonCpp 
    cppzmq protobuf log4cplus::log4cplus msgpack-cxx 
    hiredis::hiredis
    redis++::redis++_static odbc)

# add_executable(lockfree ./tests/lockfree_test.cpp ./lob.cpp)
# add_executable(redis-test ./tests/redis_test.cpp )
# target_link_libraries(lockfree msgpack-cxx  )
# target_link_libraries(redis-test hiredis::hiredis redis++::redis++_static  )

include(GNUInstallDirs )

install(TARGETS cpposmerge
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
