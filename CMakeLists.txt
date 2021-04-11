set(SRCS
    Epoll.cpp	
    channel.cpp
    Eventloop.cpp
    main.cpp
    otherFun.cpp
    Task.cpp
    ThreadPool.cpp
    Httpdata.cpp
    Log.cpp
    LogFile.cpp
    TimeRound.cpp
    EventLoopThreadPool.cpp
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -fno-strict-aliasing -Wall -export-dynamic -pipe -D_GNU_SOURCE -D_REENTRANT -fPIC -Wno-unused-variable -pthread -Wl,--no-as-needed -ldl -m64")
add_executable(WebServerBaseCoroutine ${SRCS})

target_link_libraries(WebServerBaseCoroutine /MR-Wuhaha/WebServerBaseCoroutine/WebServerBaseCoroutine/libco/build/libcolib.so)

