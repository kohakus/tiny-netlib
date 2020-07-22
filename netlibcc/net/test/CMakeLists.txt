add_executable(test_EventLoop test_EventLoop.cc)
target_link_libraries(test_EventLoop
    netlibcc_core
    netlibcc_net
)

add_executable(test_TimerContainer test_TimerContainer.cc)
target_link_libraries(test_TimerContainer
    netlibcc_core
    netlibcc_net
)