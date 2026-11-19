#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
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

/// Line and column (both starting at 1) of a byte offset in the text. An offset
/// at or past the end gives the position just after the last character.
inline TokenLocation get_offset_position(std::string_view whole_file,
                                         size_t offset) {
    size_t line_number = 1;
    size_t column_number = 1;
    for (size_t i = 0; i < offset && i < whole_file.size(); ++i) {
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

/// Thrown for malformed input. A std::runtime_error, so code that catches
/// that (or std::exception) keeps working; what() includes the position.
class ParseError : public std::runtime_error {
public:
    ParseError(const std::string &message, TokenLocation location)
        : std::runtime_error(message + " (line " +
                             std::to_string(location.line_number) +
                             ", column " +
                             std::to_string(location.column_number) + ")")
        , _message(message)
        , _location(location) {}

    /// The description without the position
    const std::string &message() const {
        return _message;
    }
    size_t line() const {
        return _location.line_number;
    }
    size_t column() const {
        return _location.column_number;
    }

private:
    std::string _message;
    TokenLocation _location;
};

inline TokenLocation get_token_position(const Token &token,
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
