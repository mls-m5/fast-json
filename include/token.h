
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
