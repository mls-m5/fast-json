
#include "jsonnode.h"
#include "token.h"
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
                    _current_token = {TokenType::BEGIN_OBJECT, "{"};
                    ++_depth;
                    _pos++;
                    return;
                case '}':
                    _current_token = {TokenType::END_OBJECT, "}"};
                    --_depth;
                    _pos++;
                    return;
                case '[':
                    _current_token = {TokenType::BEGIN_ARRAY, "["};
                    _pos++;
                    return;
                case ']':
                    _current_token = {TokenType::END_ARRAY, "]"};
                    _pos++;
                    --_depth;
                    return;
                case ':':
                    _current_token = {TokenType::COLON, ":"};
                    _pos++;
                    return;
                case ',':
                    _current_token = {TokenType::COMMA, ","};
                    _pos++;
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
                    else {
                        throw std::runtime_error("Unterminated string");
                    }
                    break;
                }
                default: {
                    if (isdigit(c) || c == '-' || c == '+') {
                        size_t start = _pos;
                        while (_pos < _input.size() &&
                               (isdigit(_input[_pos]) || _input[_pos] == '.')) {
                            _pos++;
                        }
                        std::string_view value(_input.data() + start,
                                               _pos - start);
                        _current_token = {TokenType::NUMBER, value};
                        return;
                    }
                    else if (isalpha(c)) {
                        size_t start = _pos;
                        while (_pos < _input.size() && isalpha(_input[_pos])) {
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
                            throw std::runtime_error("Unexpected value");
                        }
                    }
                    else {
                        throw std::runtime_error("Unexpected character");
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

JsonNode *parse_recursive(Tokenizer::const_iterator &it,
                          std::vector<JsonNode> &nodes,
                          int &current_index) {
    auto &token = *it;
    auto &current_node = nodes.at(current_index);

    switch (token.type) {
    case TokenType::BEGIN_OBJECT:
    case TokenType::BEGIN_ARRAY: {
        //        ++current_index;
        current_node = JsonNode{token};

        JsonNode *previous = nullptr;

        ++it;

        for (; it != Tokenizer::const_iterator{}; ++it) {
            if (it->type == TokenType::END_OBJECT ||
                it->type == TokenType::END_ARRAY) {
                return &current_node;
            }

            ++current_index;
            if (it->type == TokenType::COLON) {
                if (!previous) {
                    throw std::runtime_error{"unexpected colon"};
                }
                previous->value(Token{TokenType::KEY, previous->value().value});
                ++it;
                auto value = parse_recursive(it, nodes, current_index);
                previous->children(value);
            }
            else {
                auto new_node = parse_recursive(it, nodes, current_index);
                if (!new_node) {
                    continue;
                }
                if (previous) {
                    previous->next(new_node);
                }
                else {
                    current_node.children(new_node);
                }
                previous = new_node;
            }
        }
        return &current_node;
        break;
    }
    case TokenType::END_ARRAY:
    case TokenType::END_OBJECT:
    case TokenType::COLON:
    case TokenType::KEY:
    case TokenType::INVALID:
        throw std::runtime_error{"unexpected character " +
                                 std::string{token.value}};
        break;
    case TokenType::STRING:
    case TokenType::NUMBER:
    case TokenType::BOOLEAN:
    case TokenType::NULL_VALUE: {
        current_node = JsonNode{token};
        return &current_node;
        break;
    }
    case TokenType::COMMA:
        return nullptr;
        break;
    }

    return nullptr;
};

} // namespace json_internal

std::vector<JsonNode> parse_json(std::string_view input) {
    using namespace json_internal;
    size_t num_nodes = 0;

    // First pass: Count the number of JSON nodes
    for (auto &token : Tokenizer{input}) {
        if (token.type == TokenType::BEGIN_OBJECT ||
            token.type == TokenType::BEGIN_ARRAY ||
            token.type == TokenType::STRING ||
            token.type == TokenType::NUMBER ||
            token.type == TokenType::BOOLEAN ||
            token.type == TokenType::COLON ||
            token.type == TokenType::NULL_VALUE) {
            ++num_nodes;
        }
    }

    std::vector<JsonNode> nodes(num_nodes);
    int current_index = 0;

    auto it = Tokenizer{input}.begin();
    parse_recursive(it, nodes, current_index);

    return nodes;
}

} // namespace json
