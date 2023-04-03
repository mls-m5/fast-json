
#include "jsonnode.h"
#include "token.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

template <typename TokenProcessor>
void tokenize(std::string_view input, TokenProcessor process_token) {
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
            }
            else {
                throw std::runtime_error("Unterminated string");
            }
            break;
        }
        default: {
            if (isdigit(c) || c == '-' || c == '+') {
                size_t start = pos;
                while (pos < input.size() &&
                       (isdigit(input[pos]) || input[pos] == '.')) {
                    pos++;
                }
                std::string_view value(input.data() + start, pos - start);
                process_token({TokenType::NUMBER, value});
            }
            else if (isalpha(c)) {
                size_t start = pos;
                while (pos < input.size() && isalpha(input[pos])) {
                    pos++;
                }
                std::string_view value(input.data() + start, pos - start);
                if (value == "true" || value == "false") {
                    process_token({TokenType::BOOLEAN, value});
                }
                else if (value == "null") {
                    process_token({TokenType::NULL_VALUE, value});
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
}

std::vector<JsonNode> parse_json(std::string_view input) {
    size_t num_nodes = 0;

    // First pass: Count the number of JSON nodes
    tokenize(input, [&num_nodes](const Token &token) {
        if (token.type == TokenType::BEGIN_OBJECT ||
            token.type == TokenType::BEGIN_ARRAY ||
            token.type == TokenType::STRING ||
            token.type == TokenType::NUMBER ||
            token.type == TokenType::BOOLEAN ||
            token.type == TokenType::COLON ||
            token.type == TokenType::NULL_VALUE) {
            num_nodes++;
        }
    });

    num_nodes += 100; // TODO: Fix this

    std::vector<JsonNode> nodes(num_nodes);
    std::vector<size_t> num_children(num_nodes, 0);

    // Second pass: Create JSON nodes and link them
    size_t current_index = 0;
    std::vector<size_t> node_stack = {0};

    tokenize(input,
             [&nodes, &num_children, &current_index, &node_stack](
                 const Token &token) {
                 auto &current_node = nodes.at(current_index);

                 auto add_child = [&node_stack,
                                   &nodes,
                                   current_index,
                                   &current_node]() {
                     auto previous_index = node_stack.back();
                     if (previous_index != current_index) {
                         auto &previous = nodes.at(previous_index);
                         previous.next(&current_node);
                     }
                     else {
                         if (node_stack.size() > 1) {
                             auto &parent =
                                 nodes[node_stack.at(node_stack.size() - 2)];
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
                         previous.value(
                             Token{TokenType::KEY, previous.value().value});
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
                         return;
                     }
                     node_stack.pop_back();
                     break;
                 }
                 default:
                     break;
                 }
             });

    return nodes;
}
