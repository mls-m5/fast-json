#include "json.h"

int main() {
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

    return 0;
}
