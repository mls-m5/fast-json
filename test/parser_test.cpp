#include "fast-json/json.h"
#include "mls-unit-test/unittest.h"
#include <random>
#include <string>
#include <vector>

namespace {
// True when the text parses; false when parsing throws. Either way it must
// return: a hang or a crash fails the whole run.
bool parses(std::string_view text) {
    try {
        json::parse_json(text);
        return true;
    }
    catch (const std::exception &) {
        return false;
    }
}
// Touches every node so that a malformed tree would be noticed.
void walk(const json::JsonNode &node, int depth = 0) {
    if (depth > 64) {
        return;
    }
    switch (node.value().type) {
    case json::TokenType::BEGIN_OBJECT:
        node.visit([&](std::string_view, const json::JsonNode &child) {
            walk(child, depth + 1);
        });
        break;
    case json::TokenType::BEGIN_ARRAY:
        for (const auto &child : node) {
            walk(child, depth + 1);
        }
        break;
    default:
        break;
    }
}
const std::vector<std::string> samples = {
    R"({"name":"x\"y","list":[1,-2,3.5e2,true,false,null],"o":{"k":[{},[]]}})",
    "{\n  \"a\": [ 1, 2, {\"b\": \"c\\\\\"} ],\n  \"d\": { }\n}",
    R"([[1,2],{"a":"b"},"s",[],{},-0.5,1E-2])",
    R"([{"x":[{"y":[{"z":null}]}]}])",
};
} // namespace

TEST_SUIT_BEGIN(json_Parser)

TEST_CASE("NegativeNumbers") {
    auto root = json::parse_json("[-5, -0, -1.5e-3]");
    auto it = root->begin();
    EXPECT_EQ(it->number<int>(), -5);
    ++it;
    EXPECT_EQ(it->number<int>(), 0);
    ++it;
    EXPECT_EQ(it->number<double>(), -1.5e-3);

    auto object = json::parse_json(R"({"a": -5, "b": -12345678901})");
    EXPECT_EQ(object["a"].number<int>(), -5);
    EXPECT_EQ(object["b"].number<long long>(), -12345678901LL);
}

TEST_CASE("FractionsAndExponents") {
    auto root = json::parse_json("[1.5, 2e3, 1E-2, 0.25, 1e+2, 10]");
    auto it = root->begin();
    EXPECT_EQ(it->number<double>(), 1.5);
    ++it;
    EXPECT_EQ(it->number<double>(), 2000.0);
    ++it;
    EXPECT_EQ(it->number<double>(), 0.01);
    ++it;
    EXPECT_EQ(it->number<double>(), 0.25);
    ++it;
    EXPECT_EQ(it->number<double>(), 100.0);
    ++it;
    // Integers keep working as before, as integers and as doubles.
    EXPECT_EQ(it->number<int>(), 10);
    EXPECT_EQ(it->number<double>(), 10.0);
}

TEST_CASE("NumbersAreNotTruncatedToIntegers") {
    auto root = json::parse_json("[1.5, 2e3, -7, 4]");
    auto it = root->begin();
    bool threw = false;
    try {
        it->number<int>(); // Used to return 1.
    }
    catch (const std::invalid_argument &) {
        threw = true;
    }
    EXPECT(threw);
    threw = false;
    try {
        (++it)->number<long>(); // 2e3 is not an integer lexeme.
    }
    catch (const std::invalid_argument &) {
        threw = true;
    }
    EXPECT(threw);
    threw = false;
    try {
        (++it)->number<unsigned>(); // Negative into unsigned.
    }
    catch (const std::invalid_argument &) {
        threw = true;
    }
    EXPECT(threw);
    EXPECT_EQ((++it)->number<unsigned>(), 4u);
}

TEST_CASE("TruncatedInputIsAnError") {
    for (auto text : {"{",
                      "[",
                      "{\"a\"",
                      "{\"a\":",
                      "{\"a\":1",
                      "{\"a\":1,",
                      "{\"a\":\"x",
                      "{\"a\":[1,2",
                      "{\"a\":[1,2}",
                      "[1",
                      "[1,",
                      "[[1],",
                      "\"abc",
                      "",
                      "   "}) {
        EXPECT(!parses(text));
    }
}

TEST_CASE("MissingOrMisplacedCommasAndColons") {
    for (auto text : {R"({"a":1 "b":2})",
                      "[1 2]",
                      R"(["a" "b"])",
                      "[[1] [2]]",
                      R"({"a":1 })"
                      R"({"b":2})",
                      "[1,,2]",
                      "[,1]",
                      "[,]",
                      "{,}",
                      R"({,"a":1})",
                      R"({"a":1,,"b":2})",
                      R"({"a" 1})",
                      R"({"a":})",
                      R"({"a"})",
                      "{1:2}",
                      "[1:2]",
                      ":",
                      ","}) {
        EXPECT(!parses(text));
    }
}

TEST_CASE("TrailingCommaStaysAccepted") {
    // A decision by the owner: a comma before the closing bracket is fine.
    EXPECT(parses("[1,2,]"));
    EXPECT(parses(R"({"a":1,})"));
    EXPECT(parses(R"({"a":[1,],})"));
    auto root = json::parse_json("[1,2,]");
    int count = 0;
    for (const auto &item : *root) {
        (void)item;
        ++count;
    }
    EXPECT_EQ(count, 2);
}

TEST_CASE("DataAfterTheDocumentIsAnError") {
    for (auto text : {"{} x", "{}{}", "[] 1", "1 2", R"("a" "b")", "[]]", "{}}"}) {
        EXPECT(!parses(text));
    }
}

TEST_CASE("MismatchedBrackets") {
    for (auto text : {"[}", "{]", R"({"a":1])", "[1}", "]", "}", R"({"a":[}])"}) {
        EXPECT(!parses(text));
    }
}

TEST_CASE("MalformedNumbers") {
    for (auto text : {"[01]", "[1.]", "[.5]", "[+1]", "[-]", "[1e]", "[1e+]",
                      "[--1]", "[1.2.3]", "[1-2]", "[0x10]", "[12abc]"}) {
        EXPECT(!parses(text));
    }
}

TEST_CASE("ValidDocumentsStillParse") {
    for (auto text : {"{}", "[]", "[[]]", "{\"a\":{}}", " \t\r\n[ 1 , 2 ]\n",
                      "5", "-5", "\"s\"", "true", "false", "null",
                      R"({"a":null,"b":[true,false],"c":{"d":"e"}})"}) {
        EXPECT(parses(text));
    }
    for (const auto &text : samples) {
        EXPECT(parses(text));
    }
    auto root = json::parse_json(
        R"({"a": [1, {"b": "c"}], "d": {"e": true}, "f": "g\nh"})");
    EXPECT_EQ(root["a"].front().number<int>(), 1);
    EXPECT_EQ(root["a"].front().next()->at("b").str(), "c");
    EXPECT(root["d"]["e"].boolean());
    EXPECT_EQ(root["f"].str(), "g\nh");
}

TEST_CASE("ErrorsCarryLineAndColumn") {
    auto position = [](std::string_view text) {
        try {
            json::parse_json(text);
        }
        catch (const json::ParseError &error) {
            return std::pair<size_t, size_t>{error.line(), error.column()};
        }
        return std::pair<size_t, size_t>{0, 0};
    };
    // The offending token: the opening quote of "b".
    EXPECT(position("{\n\"a\":1\n  \"b\":2}") == (std::pair<size_t, size_t>{3, 3}));
    // Truncation is reported one past the last character.
    EXPECT(position("{\"a\":1") == (std::pair<size_t, size_t>{1, 7}));
    EXPECT(position("[1,\n 2,\n") == (std::pair<size_t, size_t>{3, 1}));
    EXPECT(position("[1] x") == (std::pair<size_t, size_t>{1, 5}));
    EXPECT(position("[1,\n @]") == (std::pair<size_t, size_t>{2, 2}));
    EXPECT(position("\"abc") == (std::pair<size_t, size_t>{1, 1}));
    // ParseError is a std::runtime_error, so existing handlers keep working.
    bool caught = false;
    try {
        json::parse_json("[");
    }
    catch (const std::runtime_error &) {
        caught = true;
    }
    EXPECT(caught);
}

TEST_CASE("EveryTruncationOfAValidDocumentIsAnError") {
    // The samples are objects or arrays ending in their closing bracket, so no
    // proper prefix is a complete value.
    for (const auto &sample : samples) {
        EXPECT(parses(sample));
        for (size_t length = 0; length < sample.size(); ++length) {
            EXPECT(!parses(std::string_view{sample}.substr(0, length)));
        }
    }
}

TEST_CASE("RandomMutationsTerminateAndNeverCrash") {
    const std::string alphabet = "{}[]\",:-+.eE0123456789tfnul\\ \n\t\x01\xff";
    std::mt19937 random{12345};
    size_t accepted = 0;
    for (int i = 0; i < 4000; ++i) {
        std::string text = samples[random() % samples.size()];
        for (int edits = 1 + random() % 3; edits > 0; --edits) {
            const auto at = random() % (text.size() + 1);
            switch (random() % 4) {
            case 0:
                if (!text.empty()) {
                    text.erase(std::min(at, text.size() - 1), 1);
                }
                break;
            case 1:
                text.insert(at, 1, alphabet[random() % alphabet.size()]);
                break;
            case 2:
                if (!text.empty()) {
                    text[std::min(at, text.size() - 1)] =
                        alphabet[random() % alphabet.size()];
                }
                break;
            default:
                text.resize(at);
            }
        }
        try {
            auto root = json::parse_json(text);
            walk(*root);
            ++accepted;
        }
        catch (const std::exception &) {
        }
    }
    EXPECT(accepted > 0);
    EXPECT(accepted < 4000);
}

TEST_SUIT_END
