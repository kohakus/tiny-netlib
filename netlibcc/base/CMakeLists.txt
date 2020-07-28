project(netlibcc_base)

include(CheckFunctionExists)
check_function_exists(accept4 HAVE_ACCEPT4)

if(NOT HAVE_ACCEPT4)
    message("accept4 not found")
    set_source_files_properties(SockUtils.cc PROPERTIES COMPILE_FLAGS "-DNO_ACCEPT4")
endif()

add_library(${PROJECT_NAME}
    Buffer.cc
    InetAddr.cc
    Socket.cc
    SockOps.cc
    SockUtils.cc
    Timer.cc
)

target_link_libraries(${PROJECT_NAME}
    netlibcc_core
)

if(NETLIBCC_BUILD_TESTS)
    add_subdirectory(test)
endif()