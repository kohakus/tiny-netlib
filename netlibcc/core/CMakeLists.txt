project(netlibcc_core)

add_library(${PROJECT_NAME}
    AsyncLog.cc
    Condition.cc
    CountDownLatch.cc
    FixedBuffer.cc
    FileAppender.cc
    LogStream.cc
    LogFile.cc
    Logger.cc
    Thread.cc
    ThreadPool.cc
    TimeAnchor.cc
    ThisThread.cc
)

target_link_libraries(${PROJECT_NAME}
    pthread
    rt
    fmt::fmt
)

if(NETLIBCC_BUILD_TESTS)
    add_subdirectory(test)
endif()