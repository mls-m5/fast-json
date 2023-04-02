
#pragma once
#include <string_view>

enum class TokenType {
    BEGIN_OBJECT,
    END_OBJECT,
    BEGIN_ARRAY,
    END_ARRAY,
    KEY,
    STRING,
    NUMBER,
    BOOLEAN,
    NULL_VALUE,
    COLON,
    COMMA
};

struct Token {
    TokenType type;
    std::string_view value;
};