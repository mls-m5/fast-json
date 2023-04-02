
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

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

template <typename TokenProcessor>
void tokenize(const std::string& input, TokenProcessor process_token) {
    size_t pos = 0;

    while (pos < input.size()) {
        char c = input[pos];

        switch (c) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                pos++; // Ignore whitespace
                break;
            case '{':
                process_token({TokenType::BEGIN_OBJECT, "{"});
                pos++;
                break;
            case '}':
                process_token({TokenType::END_OBJECT, "}"});
                pos++;
                break;
            case '[':
                process_token({TokenType::BEGIN_ARRAY, "["});
                pos++;
                break;
            case ']':
                process_token({TokenType::END_ARRAY, "]"});
                pos++;
                break;
            case ':':
                process_token({TokenType::COLON, ":"});
                pos++;
                break;
            case ',':
                process_token({TokenType::COMMA, ","});
                pos++;
                break;
            case '\"': {
                size_t start = pos;
                pos++; // Move past the opening quote
                while (pos < input.size() && input[pos] != '\"') {
                    if (input[pos] == '\\') {
                        pos++; // Skip the escape character
                    }
                    pos++;
                }
                if (pos < input.size() && input[pos] == '\"') {
                    pos++; // Move past the closing quote
                    std::string_view value(input.data() + start, pos - start);
                    process_token({TokenType::STRING, value});
                } else {
                    throw std::runtime_error("Unterminated string");
                }
                break;
            }
            default: {
                if (isdigit(c) || c == '-' || c == '+') {
                    size_t start = pos;
                    while (pos < input.size() && (isdigit(input[pos]) || input[pos] == '.')) {
                        pos++;
                    }
                    std::string_view value(input.data() + start, pos - start);
                    process_token({TokenType::NUMBER, value});
                } else if (isalpha(c)) {
                    size_t start = pos;
                    while (pos < input.size() && isalpha(input[pos])) {
                        pos++;
                    }
                    std::string_view value(input.data() + start, pos - start);
                    if (value == "true" || value == "false") {
                        process_token({TokenType::BOOLEAN, value});
                    } else if (value == "null") {
                        process_token({TokenType::NULL_VALUE, value});
                    } else {
                        throw std::runtime_error("Unexpected value");
                    }
                } else {
                    throw std::runtime_error("Unexpected character");
                }
            }
        }
    }
}

class JsonNode {
public:
    JsonNode(const Token& value) : _value(value) {}

    void children(JsonNode* children, size_t numChildren) {
        _children = children;
        _numChildren = numChildren;
    }

    const Token& value() const {
        return _value;
    }

    const JsonNode* children() const {
        return _children;
    }

    size_t numChildren() const {
        return _numChildren;
    }

private:
    Token _value;
    JsonNode* _children = nullptr;
    size_t _numChildren = 0;
};