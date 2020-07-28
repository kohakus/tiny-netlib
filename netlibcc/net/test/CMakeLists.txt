add_executable(test_EventLoop test_EventLoop.cc)
target_link_libraries(test_EventLoop
    netlibcc_net
)

add_executable(test_TimerContainer test_TimerContainer.cc)
target_link_libraries(test_TimerContainer
    netlibcc_net
)

add_executable(test_IOThreadPool test_IOThreadPool.cc)
target_link_libraries(test_IOThreadPool
    netlibcc_net
)

add_executable(test_Acceptor test_Acceptor.cc)
target_link_libraries(test_Acceptor
    netlibcc_net
)

add_executable(test_TCPServer test_TCPServer.cc)
target_link_libraries(test_TCPServer
    netlibcc_net
)