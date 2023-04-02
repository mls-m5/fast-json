#include "json.h"

// Test tokenizer
void printTokensTest() {
    std::string input = R"(
    {
      "name": "Alice",
      "age": 30,
      "city": "NYC",
      "active": true
    }
    )";

    tokenize(input, [](const Token &token) {
        std::cout << "Token type: " << static_cast<int>(token.type)
                  << ", value: "
                  << "Token type: " << static_cast<int>(token.type)
                  << ", value: " << token.value << std::endl;
    });
}

std::string token_type_to_string(TokenType type) {
    switch (type) {
    case TokenType::BEGIN_OBJECT:
        return "BEGIN_OBJECT";
    case TokenType::END_OBJECT:
        return "END_OBJECT";
    case TokenType::BEGIN_ARRAY:
        return "BEGIN_ARRAY";
    case TokenType::END_ARRAY:
        return "END_ARRAY";
    case TokenType::KEY:
        return "KEY";
    case TokenType::STRING:
        return "STRING";
    case TokenType::NUMBER:
        return "NUMBER";
    case TokenType::BOOLEAN:
        return "BOOLEAN";
    case TokenType::NULL_VALUE:
        return "NULL_VALUE";
        //    case TokenType::UNKNOWN:
        //        return "UNKNOWN";
    default:
        return "INVALID";
    }
}

void print_json(const std::vector<JsonNode> &nodes,
                size_t start = 0,
                int level = 0) {
    const JsonNode &node = nodes[start];
    for (int i = 0; i < level; ++i) {
        std::cout << "  ";
    }

    std::cout << token_type_to_string(node.value().type) << ": "
              << node.value().value << std::endl;

    for (size_t i = 0; i < node.numChildren(); ++i) {
        print_json(nodes, start + i + 1, level + 1);
    }
}

int main() {
    std::string example_data = R"({
        "name": "John",
        "age": 30,
        "city": "New York",
        "isStudent": false,
        "courses": ["math", "history", "chemistry"]
    })";

    std::vector<JsonNode> nodes = parse_json(example_data);
    print_json(nodes);

    return 0;
}
