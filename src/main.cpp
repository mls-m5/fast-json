// #include "json.h"
#include "fast-json/json.h"
#include "fast-json/jsonnode.h"
#include "fast-json/jsonout.h"
#include <algorithm>
#include <chrono>
#include <ostream>
#include <string_view>
#include <thread>

using namespace json;

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
    default:
        return "INVALID";
    }
}

// Test tokenizer
void printTokensTest(std::string_view input) {
    for (auto &token : json_internal::Tokenizer{input}) {
        std::cout << "Token type: " << token_type_to_string(token.type)
                  << ", value: " << token.value << std::endl;
        std::this_thread::sleep_for(
            std::chrono::milliseconds{10}); // Prevent output from being flodded
    }
}

void print_json(const JsonNode &node,
                std::string_view whole_file,
                int level = 0) {
    for (int i = 0; i < level; ++i) {
        std::cout << "  ";
    }

    TokenLocation location = get_token_position(node.value(), whole_file);
    std::cout << token_type_to_string(node.value().type) << ": "
              << node.value().value << " (Line: " << location.line_number
              << ", Column: " << location.column_number << ")" << std::endl;

    for (const auto &child : node) {
        print_json(child, whole_file, level + 1);
    }
}

void jsonOutTest() {
    std::cout << "\n----- json out ---- \n";
    std::ostream &os = std::cout;
    JsonOut json_out(os);

    json_out["int_key"] = 42;
    json_out["double_key"] = 3.14;
    json_out["string_key"] = "hello";
    json_out["bool_key"] = true;
    json_out["null_key"] = nullptr;

    {
        // Nested object
        auto nested_obj = json_out["nested_obj"];
        nested_obj["nested_int_key"] = 100;
        nested_obj["nested_bool_key"] = false;
        nested_obj["nested_string_key"] = "nested string";
    }

    {
        // Nested array
        std::vector<int> numbers = {1, 2, 3, 4, 5};
        auto nested_array = json_out["nested_array"];
        for (size_t i = 0; i < numbers.size(); ++i) {
            auto element = nested_array["array_element_" + std::to_string(i)];
            element["index"] = static_cast<int>(i);
            element["value"] = numbers[i];
        }
    }

    {
        // Deeply nested object
        auto deeply_nested_obj = json_out["deeply_nested_obj"];
        auto nested_level_1 = deeply_nested_obj["level_1"];
        auto nested_level_2 = nested_level_1["level_2"];
        auto nested_level_3 = nested_level_2["level_3"];
        nested_level_3["deep_key"] = "deep_value";
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

    std::string example_data2 = R"({
        "person": {
            "name": "John",
            "age": 30,
            "city": "New York",
            "isStudent": false,
            "courses": ["math", "history", "chemistry"],
            "contact": {
                "email": "john@example.com",
                "phone": {
                    "home": "123-456-7890",
                    "work": "987-654-3210"
                }
            },
            "hobbies": [
                {
                    "name": "running",
                    "location": ["park", "gym"]
                },
                {
                    "name": "reading",
                    "genres": ["mystery", "science fiction", "fantasy"]
                }
            ],
            "family": [
                {
                    "relation": "sister",
                    "name": "Jane",
                    "age": 32
                },
                {
                    "relation": "brother",
                    "name": "Tom",
                    "age": 28
                }
            ]
        }
    })";

    //    printTokensTest(example_data2);
    const JsonRoot root = parse_json(example_data2);
    //    auto &root = nodes.front();
    print_json(root.nodes.front(), example_data2);

    if (auto person = root->find("person")) {
        std::cout << "found 'person' in root" << std::endl;

        std::cout << "persons name is " << person->at("name").value().value;

        std::cout << "person properties:\n";
        person->visit([](std::string_view key, const JsonNode &value) {
            std::cout << "   key: " << key << " = " << value.value().value
                      << "\n";
        });

        std::cout << "int age " << person->at("age").number() << "\n";

        std::cout << "bool " << person->at("isStudent").boolean() << std::endl;
    }

    std::cout << *root;

    std::cout << "\n----- json out ---- \n";

    {
        std::ostream &os = std::cout;
        os << std::setw(2); // Set the width using std::setw

        {
            auto json_out = JsonOut{os};
            json_out["key1"] = 123;
            json_out["key2"] = "hello";
            json_out["key3"] = true;
            json_out["key4"] = nullptr;

            {
                auto nested_obj = json_out["nested_obj"];
                nested_obj["inner_key1"] = "inner_value1";
                nested_obj["inner_key2"] = false;
            }
        }
    }

    jsonOutTest();

    return 0;
}
