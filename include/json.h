
#include "jsonnode.h"
#include "token.h"
#include <iostream>
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
            token.type == TokenType::KEY || token.type == TokenType::STRING ||
            token.type == TokenType::NUMBER ||
            token.type == TokenType::BOOLEAN ||
            token.type == TokenType::NULL_VALUE) {
            num_nodes++;
        }
    });

    std::vector<JsonNode> nodes(num_nodes);
    std::vector<size_t> num_children(num_nodes, 0);

    // Second pass: Create JSON nodes and link them
    size_t current_node = 0;
    std::vector<size_t> node_stack;

    tokenize(input,
             [&nodes, &num_children, &current_node, &node_stack](
                 const Token &token) {
                 switch (token.type) {
                 case TokenType::BEGIN_OBJECT:
                 case TokenType::BEGIN_ARRAY:
                 case TokenType::KEY: {
                     auto &new_node = nodes.at(current_node);
                     new_node = JsonNode(token);
                     auto next = &nodes.at(current_node + 1);
                     new_node.children(next);

                     if (!node_stack.empty()) {
                         auto &parent = nodes[node_stack.back()];
                         parent.numChildren(parent.numChildren() + 1);
                     }

                     node_stack.push_back(current_node);

                     ++current_node;
                     break;
                 }
                 case TokenType::STRING:
                 case TokenType::NUMBER:
                 case TokenType::BOOLEAN:
                 case TokenType::NULL_VALUE: {
                     nodes[current_node] = JsonNode(token);
                     auto &parent = nodes[node_stack.back()];
                     parent.numChildren(parent.numChildren() + 1);
                     ++current_node;
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
