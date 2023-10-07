
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
