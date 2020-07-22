project(netlibcc_net)

add_library(${PROJECT_NAME}
    Channel.cc
    EPoller.cc
    EventLoop.cc
    IOThread.cc
    TimerContainer.cc
)

target_link_libraries(${PROJECT_NAME}
    netlibcc_core
    netlibcc_base
)

if(NETLIBCC_BUILD_TESTS)
    add_subdirectory(test)
endif()