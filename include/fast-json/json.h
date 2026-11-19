#pragma once

#include "jsonnode.h"
#include "token.h"
#include <filesystem>
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace json {

namespace json_internal {

class Tokenizer {
public:
    class const_iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = Token;
        using difference_type = std::ptrdiff_t;
        using pointer = const Token *;
        using reference = const Token &;

        const_iterator()
            : _input("")
            , _pos(0) {}

        explicit const_iterator(std::string_view input)
            : _input(input)
            , _pos(0) {
            advance();
        }

        reference operator*() const {
            return _current_token;
        }
        pointer operator->() const {
            return &_current_token;
        }

        const_iterator &operator++() {
            advance();
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator tmp(*this);
            operator++();
            return tmp;
        }

        bool operator==(const const_iterator &other) const {
            return (_input.empty() && other._input.empty()) ||
                   (_input.data() == other._input.data() && _pos == other._pos);
        }

        bool operator!=(const const_iterator &other) const {
            return !(*this == other);
        }

    private:
        [[noreturn]] void fail(const std::string &message,
                               size_t offset) const {
            throw ParseError(message, get_offset_position(_input, offset));
        }

        static bool is_digit(char c) {
            return c >= '0' && c <= '9';
        }

        bool digit_at(size_t pos) const {
            return pos < _input.size() && is_digit(_input[pos]);
        }

        // -?(digits)(.digits)?([eE][+-]?digits)?  No leading zeros, no '+'.
        void read_number() {
            const size_t start = _pos;
            if (_input[_pos] == '-') {
                _pos++;
            }
            if (!digit_at(_pos)) {
                fail("Invalid number", start);
            }
            if (_input[_pos] == '0' && digit_at(_pos + 1)) {
                fail("Numbers must not have leading zeros", start);
            }
            while (digit_at(_pos)) {
                _pos++;
            }
            if (_pos < _input.size() && _input[_pos] == '.') {
                _pos++;
                if (!digit_at(_pos)) {
                    fail("Invalid number", start);
                }
                while (digit_at(_pos)) {
                    _pos++;
                }
            }
            if (_pos < _input.size() &&
                (_input[_pos] == 'e' || _input[_pos] == 'E')) {
                _pos++;
                if (_pos < _input.size() &&
                    (_input[_pos] == '+' || _input[_pos] == '-')) {
                    _pos++;
                }
                if (!digit_at(_pos)) {
                    fail("Invalid number", start);
                }
                while (digit_at(_pos)) {
                    _pos++;
                }
            }
            _current_token = {TokenType::NUMBER,
                              _input.substr(start, _pos - start)};
        }

        void punctuation(TokenType type) {
            // Points into the input, so the token has a position.
            _current_token = {type, _input.substr(_pos, 1)};
            _pos++;
        }

        void advance() {
            while (_pos < _input.size()) {
                char c = _input[_pos];

                switch (c) {
                case ' ':
                case '\t':
                case '\r':
                case '\n':
                    _pos++; // Ignore whitespace
                    break;
                case '{':
                    punctuation(TokenType::BEGIN_OBJECT);
                    ++_depth;
                    return;
                case '}':
                    punctuation(TokenType::END_OBJECT);
                    --_depth;
                    return;
                case '[':
                    punctuation(TokenType::BEGIN_ARRAY);
                    return;
                case ']':
                    punctuation(TokenType::END_ARRAY);
                    --_depth;
                    return;
                case ':':
                    punctuation(TokenType::COLON);
                    return;
                case ',':
                    punctuation(TokenType::COMMA);
                    return;
                case '\"': {
                    size_t start = _pos;
                    _pos++; // Move past the opening quote
                    while (_pos < _input.size() && _input[_pos] != '\"') {
                        if (_input[_pos] == '\\') {
                            _pos++; // Skip the escape character
                        }
                        _pos++;
                    }
                    if (_pos < _input.size() && _input[_pos] == '\"') {
                        _pos++; // Move past the closing quote
                        std::string_view value(_input.data() + start,
                                               _pos - start);
                        _current_token = {TokenType::STRING,
                                          value.substr(1, value.size() - 2)};
                        return;
                    }
                    fail("Unterminated string", start);
                }
                default: {
                    const auto uc = static_cast<unsigned char>(c);
                    if (is_digit(c) || c == '-') {
                        read_number();
                        return;
                    }
                    else if (std::isalpha(uc)) {
                        size_t start = _pos;
                        while (_pos < _input.size() &&
                               std::isalpha(
                                   static_cast<unsigned char>(_input[_pos]))) {
                            _pos++;
                        }
                        std::string_view value(_input.data() + start,
                                               _pos - start);
                        if (value == "true" || value == "false") {
                            _current_token = {TokenType::BOOLEAN, value};
                            return;
                        }
                        else if (value == "null") {
                            _current_token = {TokenType::NULL_VALUE, value};
                            return;
                        }
                        else {
                            fail("Unexpected value " + std::string{value},
                                 start);
                        }
                    }
                    else {
                        fail("Unexpected character", _pos);
                    }
                }
                }
            }

            *this = const_iterator{};
        }

        std::string_view _input;
        size_t _pos;
        Token _current_token;
        int _depth = 0;
    };

    explicit Tokenizer(std::string_view input)
        : _input(input) {}

    const_iterator begin() const {
        return const_iterator(_input);
    }

    static const_iterator end() {
        return const_iterator();
    }

private:
    std::string_view _input;
};

/// Offset of a token in the input (a string token excludes its quotes).
inline size_t token_offset(const Token &token, std::string_view input) {
    size_t offset = static_cast<size_t>(token.value.data() - input.data());
    return token.type == TokenType::STRING && offset > 0 ? offset - 1 : offset;
}

/// Checks the structure of the whole document without recursion and returns
/// the number of nodes it needs: one for every object, array, key and value.
/// A comma before a closing bracket is accepted; everything else that is not
/// JSON (truncation, a missing or extra comma, text after the document,
/// mismatched brackets) is a ParseError.
inline size_t validate(std::string_view input) {
    enum class Expect {
        Value,        // The document, or after a colon
        ArrayValue,   // After '[' or ',': a value or ']'
        ObjectKey,    // After '{' or ',': a key or '}'
        Colon,        // After a key
        AfterValue,   // After a value: ',' or the closing bracket
    };
    std::vector<bool> objects; // The open containers, true for an object
    Expect expect = Expect::Value;
    bool done = false;
    size_t count = 0;

    auto fail = [&](const std::string &message, size_t offset) {
        throw ParseError(message, get_offset_position(input, offset));
    };

    for (auto &token : Tokenizer{input}) {
        const size_t at = token_offset(token, input);
        if (done) {
            fail("Unexpected data after the document", at);
        }
        auto finish_value = [&] {
            if (objects.empty()) {
                done = true;
            }
            expect = Expect::AfterValue;
        };
        auto close = [&](bool object) {
            if (objects.empty() || objects.back() != object) {
                fail(std::string{"Unexpected '"} + std::string{token.value} +
                         "'",
                     at);
            }
            objects.pop_back();
            finish_value();
        };
        switch (token.type) {
        case TokenType::BEGIN_OBJECT:
        case TokenType::BEGIN_ARRAY:
            if (expect != Expect::Value && expect != Expect::ArrayValue) {
                fail("Expected " + std::string{expect == Expect::AfterValue
                                                   ? "',' or a closing bracket"
                                               : expect == Expect::Colon
                                                   ? "':'"
                                                   : "a string key"},
                     at);
            }
            ++count;
            objects.push_back(token.type == TokenType::BEGIN_OBJECT);
            expect = token.type == TokenType::BEGIN_OBJECT
                         ? Expect::ObjectKey
                         : Expect::ArrayValue;
            break;
        case TokenType::STRING:
            if (expect == Expect::ObjectKey) {
                ++count; // A key
                expect = Expect::Colon;
                break;
            }
            [[fallthrough]];
        case TokenType::NUMBER:
        case TokenType::BOOLEAN:
        case TokenType::NULL_VALUE:
            if (expect != Expect::Value && expect != Expect::ArrayValue) {
                fail(expect == Expect::AfterValue
                         ? std::string{objects.back() ? "Expected ',' or '}'"
                                                      : "Expected ',' or ']'"}
                     : expect == Expect::Colon ? "Expected ':'"
                                               : "Expected a string key",
                     at);
            }
            ++count;
            finish_value();
            break;
        case TokenType::COLON:
            if (expect != Expect::Colon) {
                fail("Unexpected ':'", at);
            }
            expect = Expect::Value;
            break;
        case TokenType::COMMA:
            if (expect != Expect::AfterValue) {
                fail("Unexpected ','", at);
            }
            expect = objects.back() ? Expect::ObjectKey : Expect::ArrayValue;
            break;
        case TokenType::END_OBJECT:
            if (expect != Expect::ObjectKey && expect != Expect::AfterValue) {
                fail("Unexpected '}'", at);
            }
            close(true);
            break;
        case TokenType::END_ARRAY:
            if (expect != Expect::ArrayValue && expect != Expect::AfterValue) {
                fail("Unexpected ']'", at);
            }
            close(false);
            break;
        case TokenType::KEY:
        case TokenType::INVALID:
            fail("Unexpected token", at);
        }
    }

    if (!done) {
        fail("Unexpected end of input", input.size());
    }
    return count;
}

/// Builds the node tree for a document that validate() accepted, without
/// recursion. `nodes` must have exactly the size validate() returned, because
/// the nodes point at each other.
inline void build(std::string_view input, std::vector<JsonNode> &nodes) {
    struct Frame {
        JsonNode *container;
        JsonNode *last = nullptr;        // The last child added
        JsonNode *pending_key = nullptr; // An object key waiting for its value
    };
    std::vector<Frame> stack;
    size_t next_node = 0;

    // Links a new value node into the open container.
    auto attach = [&](JsonNode *node) {
        if (stack.empty()) {
            return;
        }
        auto &frame = stack.back();
        if (frame.pending_key) {
            frame.pending_key->children(node);
            frame.pending_key = nullptr;
            return;
        }
        if (frame.last) {
            frame.last->next(node);
        }
        else {
            frame.container->children(node);
        }
        frame.last = node;
    };

    for (auto &token : Tokenizer{input}) {
        switch (token.type) {
        case TokenType::BEGIN_OBJECT:
        case TokenType::BEGIN_ARRAY: {
            auto *node = &nodes.at(next_node++);
            *node = JsonNode{token};
            attach(node);
            stack.push_back({node});
            break;
        }
        case TokenType::END_OBJECT:
        case TokenType::END_ARRAY:
            stack.pop_back();
            break;
        case TokenType::STRING:
        case TokenType::NUMBER:
        case TokenType::BOOLEAN:
        case TokenType::NULL_VALUE: {
            auto *node = &nodes.at(next_node++);
            const bool is_key = token.type == TokenType::STRING &&
                                !stack.empty() &&
                                stack.back().container->value().type ==
                                    TokenType::BEGIN_OBJECT &&
                                !stack.back().pending_key;
            if (is_key) {
                *node = JsonNode{Token{TokenType::KEY, token.value}};
                auto &frame = stack.back();
                if (frame.last) {
                    frame.last->next(node);
                }
                else {
                    frame.container->children(node);
                }
                frame.last = node;
                frame.pending_key = node;
            }
            else {
                *node = JsonNode{token};
                attach(node);
            }
            break;
        }
        default:
            break; // Colons and commas carry no node
        }
    }
}
} // namespace json_internal

/// A container for the result
struct JsonRoot {
    std::vector<JsonNode> nodes;

    const JsonNode *operator->() const {
        return &nodes.front();
    }

    const JsonNode &operator*() const {
        return nodes.front();
    }

    const JsonNode &operator[](std::string_view name) const {
        return nodes.front().at(name);
    }
};

/// Parses a complete document. Throws ParseError (a std::runtime_error) for
/// anything that is not JSON, with the line and column of the problem.
inline JsonRoot parse_json(std::string_view input) {
    using namespace json_internal;

    std::vector<JsonNode> nodes(validate(input));
    build(input, nodes);

    return {std::move(nodes)};
}

} // namespace json
