
#include "fast-json/jsonout.h"
#include "mls-unit-test/unittest.h"
// #include <iostream>
// #include <sstream>

TEST_SUIT_BEGIN(json_JsonOut)

TEST_CASE("BasicTypes") {
    auto out = json::JsonOut{std::cerr};
    out["hello"] = 10;
    out["there"] = "you!";
    out["boolean"] = true;
}

TEST_CASE("array_test") {
    {
        auto out = json::JsonOut{std::cerr};

        {
            auto a = out.push_back();

            a["hello"] = 10;
            a["there"] = "you!";
        }

        {
            auto b = out.push_back();
            b["boolean"] = true;
        }

        out.push_back() = 10;
        out.push_back(20);
    }

    std::cerr << "\ntest 2\n";

    {
        auto out = json::JsonOut{std::cerr};

        {
            auto a = out["a"];
            a.push_back() = 20;
        }
        { out["b"].push_back() = 10; }
    }
}

TEST_CASE("InlineOutput") {
    {
        auto out = json::JsonOut{std::cerr, -1};
        out["hello"] = 10;
        out["there"] = "you!";
        out["boolean"] = true;
    }
    std::cerr << "\n";
    std::cerr << "hello\n";
}

TEST_SUIT_END
