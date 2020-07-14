add_executable(test_Thread test_Thread.cc)
target_link_libraries(test_Thread netlibcc_core)

add_executable(test_TimeAnchor test_TimeAnchor.cc)
target_link_libraries(test_TimeAnchor netlibcc_core)

add_executable(test_Mutex test_Mutex.cc)
target_link_libraries(test_Mutex netlibcc_core)

add_executable(test_fmt test_fmt.cc)
target_link_libraries(test_fmt fmt::fmt)

add_executable(test_LogStream test_LogStream.cc)
target_link_libraries(test_LogStream
    netlibcc_core
    Catch2::Test
)
add_test(NAME test_LogStream COMMAND test_LogStream)

add_executable(bench_LogStream bench_LogStream.cc)
target_link_libraries(bench_LogStream netlibcc_core)

add_executable(test_ThreadPool test_ThreadPool.cc)
target_link_libraries(test_ThreadPool netlibcc_core)

add_executable(test_LogFile test_LogFile.cc)
target_link_libraries(test_LogFile netlibcc_core)

add_executable(test_AsyncLog test_AsyncLog.cc)
target_link_libraries(test_AsyncLog netlibcc_core)