add_executable(test_InetAddr test_InetAddr.cc)
target_link_libraries(test_InetAddr
    netlibcc_base
    Catch2::Test
)
add_test(NAME test_InetAddr COMMAND test_InetAddr)