#include "fast-json/json.h"
#include "fast-json/jsonnode.h"
#include "fast-json/jsonout.h"
#include "fast-json/serialize.h"
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

struct Hello1 {
    int hello = 10;
    std::string there = "yo!";

    FAST_JSON_SERIALIZE_INTRUSIVE(hello, there);
};

// Just make sure that the variable names in the macro is generic
struct Hello2 {
    int x = 10;
    std::string str = "yo!";

    FAST_JSON_SERIALIZE_INTRUSIVE(x, str);
};

struct Hello_one_member {
    int x = {};
    FAST_JSON_SERIALIZE_INTRUSIVE(x);
};

struct Hello_five_members {
    int x = {};
    double y = {};
    std::string z;
    bool a = {};
    char b = {};

    FAST_JSON_SERIALIZE_INTRUSIVE(x, y, z, a, b);

    bool operator==(const Hello_five_members &) const = default;
};

TEST(Serialization, BasicDefinedSerialization) {
    auto hello = Hello1{};

    hello.hello = 20;
    hello.there = "stahoesuth";

    auto ss = std::stringstream{};
    {
        auto out = json::JsonOut{ss};
        out["x"] = hello;
    }

    auto str = ss.str();
    std::cerr << str;
    {
        auto helloB = Hello1{};

        auto in = json::parse_json(str);
        in["x"].get_to(helloB);

        EXPECT_EQ(helloB.hello, hello.hello);
        EXPECT_EQ(helloB.there, hello.there);
    }

    {
        auto in = json::parse_json(str);
        auto helloC = in["x"].as<Hello1>();

        in["x"].get_to(helloC);

        EXPECT_EQ(helloC.hello, hello.hello);
        EXPECT_EQ(helloC.there, hello.there);
    }
}
