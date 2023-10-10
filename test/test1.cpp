
#include "fast-json/jsonout.h"
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

TEST(json_JsonOut, BasicTypes) {
    auto out = json::JsonOut{std::cerr};
    out["hello"] = 10;
    out["there"] = "you!";
    out["boolean"] = true;
}

TEST(json_JsonOut, array_test) {
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

TEST(json_JsonOut, InlineOutput) {
    {
        auto out = json::JsonOut{std::cerr, -1};
        out["hello"] = 10;
        out["there"] = "you!";
        out["boolean"] = true;
    }
    std::cerr << "\n";
    std::cerr << "hello\n";
}
