
#pragma once
#include <string_view>

namespace json {

enum class TokenType : unsigned {
    INVALID = 0,
    BEGIN_OBJECT = 1 << 0, // 1
    END_OBJECT = 1 << 1,   // 2
    BEGIN_ARRAY = 1 << 2,  // 4
    END_ARRAY = 1 << 3,    // 8
    KEY = 1 << 4,          // 16
    STRING = 1 << 5,       // 32
    NUMBER = 1 << 6,       // 64
    BOOLEAN = 1 << 7,      // 128
    NULL_VALUE = 1 << 8,   // 256
    COLON = 1 << 9,        // 512
    COMMA = 1 << 10        // 1024
};

struct Token {
    TokenType type = TokenType::INVALID;
    std::string_view value;
};

struct TokenLocation {
    size_t line_number;
    size_t column_number;
};

TokenLocation get_token_position(const Token &token,
                                 std::string_view whole_file) {
    size_t line_number = 1;
    size_t column_number = 1;

    for (size_t i = 0; i < whole_file.size(); ++i) {
        if (whole_file.data() + i == token.value.data()) {
            break;
        }

        if (whole_file[i] == '\n') {
            line_number++;
            column_number = 1;
        }
        else {
            column_number++;
        }
    }

    return TokenLocation{line_number, column_number};
}

} // namespace json
