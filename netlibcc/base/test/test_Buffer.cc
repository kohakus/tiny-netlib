#include "netlibcc/base/Buffer.h"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

using namespace netlibcc;
using netlibcc::net::Buffer;
using std::string;

TEST_CASE("TEST Buffer Append and Retrieve") {
    Buffer buf;
    REQUIRE(buf.readableBytes() == 0);
    REQUIRE(buf.writableBytes() == Buffer::kInitSize);
    REQUIRE(buf.capacity() == Buffer::kInitSize+Buffer::kPrepend);
    REQUIRE(buf.capacity() == 2048);
    REQUIRE(buf.rpos()-buf.wpos() == 0);

    const string str(200, 'x');
    buf.append(str);
    REQUIRE(buf.readableBytes() == str.size());
    REQUIRE(buf.writableBytes() == Buffer::kInitSize - str.size());

    const string str2 = buf.retrieveAsStr(50);
    REQUIRE(str2.size() == 50);
    REQUIRE(str2 == string(50, 'x'));
    REQUIRE(buf.readableBytes() == str.size() - str2.size());
    REQUIRE(buf.writableBytes() == Buffer::kInitSize - str.size());

    buf.append(str);
    REQUIRE(buf.readableBytes() == 2*str.size() - str2.size());
    REQUIRE(buf.writableBytes() == Buffer::kInitSize - 2*str.size());

    const string str3 = buf.retrieveAllAsStr();
    REQUIRE(str3.size() == 350);
    REQUIRE(str3 == string(350, 'x'));
    REQUIRE(buf.readableBytes() == 0);
    REQUIRE(buf.writableBytes() == Buffer::kInitSize);
    REQUIRE(buf.rpos()-buf.wpos() == 0);
}

TEST_CASE("Test Buffer Grow") {
    Buffer buf;
    buf.append(string(400, 'y'));
    REQUIRE(buf.readableBytes() == 400);
    REQUIRE(buf.writableBytes() == Buffer::kInitSize - 400);

    buf.append(string(2000, 'x'));
    REQUIRE(buf.capacity() == 4096);
    REQUIRE(buf.readableBytes() == 2400);
    REQUIRE(buf.writableBytes() == 0);

    buf.append(string(1000, 'z'));
    REQUIRE(buf.capacity() == 4096);
    REQUIRE(buf.readableBytes() == 3400);
    REQUIRE(buf.writableBytes() == 0);

    buf.retrieve(2000);
    REQUIRE(buf.capacity() == 4096);
    REQUIRE(buf.readableBytes() == 1400);
    REQUIRE(buf.writableBytes() == 0);

    buf.append(string(800, 'm'));
    REQUIRE(buf.capacity() == 4096);
    REQUIRE(buf.readableBytes() == 2200);
    REQUIRE(buf.writableBytes() == 1200);
}

TEST_CASE("Test Buffer Grow Again") {
    Buffer buf;
    buf.append(string(2050, 'x'));
    REQUIRE(buf.capacity() == 4096);
    REQUIRE(buf.readableBytes() == 2050);
    REQUIRE(buf.writableBytes() == 0);

    buf.retrieve(1500);
    REQUIRE(buf.capacity() == 4096);
    REQUIRE(buf.readableBytes() == 550);
    REQUIRE(buf.writableBytes() == 0);

    buf.append(string(2050, 'y'));
    REQUIRE(buf.capacity() == 4096);
    REQUIRE(buf.readableBytes() == 2600);
    REQUIRE(buf.writableBytes() == 0);
}