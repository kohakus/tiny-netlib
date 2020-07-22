#include "netlibcc/base/InetAddr.h"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

using netlibcc::net::InetAddr;
using std::string;

TEST_CASE("Test InetAddr") {
    InetAddr addr0(1234);
    REQUIRE(addr0.toIpStr() == string("0.0.0.0"));
    REQUIRE(addr0.toIpPortStr() == string("0.0.0.0:1234"));
    REQUIRE(addr0.toPort() == 1234);

    InetAddr addr1(4321, true);
    REQUIRE(addr1.toIpStr() == string("127.0.0.1"));
    REQUIRE(addr1.toIpPortStr() == string("127.0.0.1:4321"));
    REQUIRE(addr1.toPort() == 4321);

    InetAddr addr2("1.2.3.4", 8888);
    REQUIRE(addr2.toIpStr() == string("1.2.3.4"));
    REQUIRE(addr2.toIpPortStr() == string("1.2.3.4:8888"));
    REQUIRE(addr2.toPort() == 8888);

    InetAddr addr3("255.254.253.252", 65535);
    REQUIRE(addr3.toIpStr() == string("255.254.253.252"));
    REQUIRE(addr3.toIpPortStr() == string("255.254.253.252:65535"));
    REQUIRE(addr3.toPort() == 65535);
}