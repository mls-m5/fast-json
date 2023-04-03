
#include "jsonnode.h"
#include "token.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

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
                        _current_token = {TokenType::STRING, value};
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

    const_iterator end() const {
        return const_iterator();
    }

private:
    std::string_view _input;
};

std::vector<JsonNode> parse_json(std::string_view input) {
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
            num_nodes++;
        }
    }

    num_nodes += 100; // TODO: Fix this

    std::vector<JsonNode> nodes(num_nodes);
    std::vector<size_t> num_children(num_nodes, 0);

    // Second pass: Create JSON nodes and link them
    size_t current_index = 0;
    std::vector<size_t> node_stack = {0};

    for (auto &token : Tokenizer{input}) {
        auto &current_node = nodes.at(current_index);

        auto add_child = [&node_stack, &nodes, current_index, &current_node]() {
            auto previous_index = node_stack.back();
            if (previous_index != current_index) {
                auto &previous = nodes.at(previous_index);
                previous.next(&current_node);
            }
            else {
                if (node_stack.size() > 1) {
                    auto &parent = nodes[node_stack.at(node_stack.size() - 2)];
                    parent.children(&current_node);
                }
            }
        };

        switch (token.type) {
        case TokenType::BEGIN_OBJECT:
        case TokenType::BEGIN_ARRAY: {
            current_node = JsonNode{token};

            add_child();

            node_stack.back() = current_index;
            node_stack.push_back(current_index + 1);

            ++current_index;
            break;
        }
        case TokenType::COLON: {
            auto previous_index = node_stack.back();
            if (previous_index != current_index) {
                auto &previous = nodes.at(previous_index);
                previous.value(Token{TokenType::KEY, previous.value().value});
            }
            else {
                throw std::runtime_error{"unexpected token " +
                                         std::string{token.value}};
            }

            //                     node_stack.back() = current_index;
            node_stack.push_back(current_index + 1);

            //                     ++current_index;
            break;
        }
        case TokenType::STRING:
        case TokenType::NUMBER:
        case TokenType::BOOLEAN:
        case TokenType::NULL_VALUE: {
            nodes[current_index] = JsonNode(token);

            add_child();

            node_stack.back() = current_index;
            ++current_index;
            break;
        }
        case TokenType::END_OBJECT:
        case TokenType::END_ARRAY: {
            if (node_stack.empty()) {
                return nodes; // TODO: Handle possible error if invalid
            }
            node_stack.pop_back();
            break;
        }
        default:
            break;
        }
        //             });
    }

    return nodes;
}
