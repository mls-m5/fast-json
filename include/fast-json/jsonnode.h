#pragma once

#include "token.h"
#include <charconv>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <string_view>

namespace json {

class JsonNode {
public:
    class const_iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = const JsonNode;
        using difference_type = std::ptrdiff_t;
        using pointer = const JsonNode *;
        using reference = const JsonNode &;

        const_iterator()
            : _node(nullptr) {}
        explicit const_iterator(const JsonNode *node)
            : _node(node) {}

        reference operator*() const {
            return *_node;
        }

        pointer operator->() const {
            return _node;
        }

        const_iterator &operator++() {
            if (_node) {
                _node = _node->next();
            }
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator tmp(*this);
            operator++();
            return tmp;
        }

        bool operator==(const const_iterator &other) const {
            return _node == other._node;
        }

        bool operator!=(const const_iterator &other) const {
            return !operator==(other);
        }

    private:
        const JsonNode *_node;
    };

    JsonNode(const JsonNode &) = delete;
    JsonNode(JsonNode &&) = default;
    JsonNode &operator=(const JsonNode &) = delete;
    JsonNode &operator=(JsonNode &&) = default;
    JsonNode() = default;
    JsonNode(const Token &value)
        : _value(value) {}

    const Token &value() const {
        return _value;
    }

    const Token &value(Token value) {
        return _value = value;
    }

    const JsonNode &front() const {
        if (!_children) {
            throw std::out_of_range{"no children in node"};
        }
        return *_children;
    }

    const_iterator begin() const {
        if (!_children) {
            throw std::out_of_range{"no children in node"};
        }
        return const_iterator(children());
    }

    const_iterator end() const {
        return const_iterator(nullptr);
    }
    const JsonNode *children() const {
        return _children;
    }

    void children(const JsonNode *children) {
        _children = children;
    }

    const JsonNode *next() const {
        return _next;
    }

    void next(const JsonNode *node) {
        _next = node;
    }

    const JsonNode &at(std::string_view name) const {
        if (_value.type == TokenType::KEY) {
            return _children->at(name);
        }
        validate_object();
        for (auto &child : *this) {
            child.validate_key();
            if (child.value().value == name) {
                // Note that a key actually holds it's value as a child
                return *child.children();
            }
        }
        throw std::out_of_range{"element " + std::string{name} + " not found"};
    }

    const JsonNode &operator[](std::string_view name) const {
        return at(name);
    }

    const JsonNode *find(std::string_view name) const {
        if (_value.type == TokenType::KEY) {
            return _children->find(name);
        }
        validate_object();
        for (auto it = begin(); it != end(); ++it) {
            it->validate_key();
            if (it->value().value == name) {
                return it->children();
            }
        }
        return nullptr;
    }

    // Iterato through an object and call a function for each key-value-pair
    // The first argument should be the name and the second a const
    // JsonNode&
    template <typename F>
    void visit(F f) const {
        validate_object();

        for (auto &key : *this) {
            auto &child = *key.children();
            // Note: Wierd syntax here is to have special behaviour for
            // value() for keys
            f(key._value.value, child);
        }
    }

    template <typename T = int>
    T number() const {
        validate_number();
        int out;
        const std::from_chars_result result =
            std::from_chars(_value.value.data(),
                            _value.value.data() + _value.value.size(),
                            out);
        if (result.ec == std::errc::invalid_argument ||
            result.ec == std::errc::result_out_of_range) {
            throw std::invalid_argument{"invalid number"};
        }
        return out;
    }

    bool boolean() const {
        validate_bool();
        return _value.value == "true";
    }

    std::string_view raw() const {
        validate_string();
        return _value.value;
    }

    // Get a string where the escape characters is applied
    std::string str() const {
        auto res = std::string{};

        bool escaped = false;
        for (auto c :
             raw()) { // assuming str() returns the original escaped string
            if (escaped) {
                char e = 0;
                switch (c) {
                case '\"':
                    e = '\"';
                    break; // Quotation mark
                case '\\':
                    e = '\\';
                    break; // Reverse solidus
                case '/':
                    e = '/';
                    break; // Solidus
                case 'b':
                    e = '\b';
                    break; // Backspace
                case 'f':
                    e = '\f';
                    break; // Form feed
                case 'n':
                    e = '\n';
                    break; // Newline
                case 'r':
                    e = '\r';
                    break; // Carriage return
                case 't':
                    e = '\t';
                    break; // Horizontal tab
                // add more case labels here if needed
                default:
                    // Handle error: invalid escape sequence.
                    throw std::runtime_error("Invalid escape sequence: \\" +
                                             std::string(1, c));
                }
                res.push_back(e);
                escaped = false;
            }
            else if (c == '\\') {
                escaped = true;
            }
            else {
                res.push_back(c);
            }
        }

        // Check if string ends with a single backslash
        if (escaped) {
            throw std::runtime_error("String ends with an unescaped backslash");
        }

        return res;
    }

private:
    void validate_object() const {
        if (_value.type != TokenType::BEGIN_OBJECT) {
            throw std::invalid_argument{"json entity is not of type 'object'"};
        }
    }

    void validate_key() const {
        if (_value.type != TokenType::KEY) {
            throw std::invalid_argument{"json entity is not of type 'key'"};
        }
        if (!_children) {
            throw std::invalid_argument{"key has no child!"};
        }
    }

    void validate_number() const {
        if (_value.type != TokenType::NUMBER) {
            throw std::invalid_argument{"json entity is not of type 'number'"};
        }
    }

    void validate_bool() const {
        if (_value.type != TokenType::BOOLEAN) {
            throw std::invalid_argument{"json entity is not of type 'boolean'"};
        }
    }

    void validate_string() const {
        if (_value.type != TokenType::STRING) {
            throw std::invalid_argument{"json entity is not of type 'string'"};
        }
    }

    Token _value;
    const JsonNode *_children = nullptr;
    const JsonNode *_next = nullptr;
};

inline void dump(const JsonNode &node, std::ostream &os, int indent = 0) {

    int current_width =
        os.width(); // Capture the current width set by std::setw

    if (!current_width) {
        current_width = 2;
    }

    auto print_indent = [current_width, &os](int n) {
        os << std::setw(n * current_width) << "";
    };

    switch (node.value().type) {
    case TokenType::BEGIN_OBJECT: {
        os << "{\n";
        bool first = true;
        for (const auto &child : node) {
            if (!first) {
                os << ",\n";
            }
            first = false;
            print_indent(indent + 1);
            os << std::quoted(child.value().value) << ": ";
            dump(*child.children(), os, indent + 1);
        }
        os << "\n";
        print_indent(indent);
        os << "}";
        break;
    }
    case TokenType::BEGIN_ARRAY: {
        os << "[\n";
        bool first = true;
        for (const auto &child : node) {
            if (!first) {
                os << ",\n";
            }
            first = false;
            print_indent(indent + 1);
            dump(child, os, indent + 1);
        }
        os << "\n";
        print_indent(indent);
        os << "]";
        break;
    }
    case TokenType::STRING:
        os << std::quoted(node.raw());
        break;
    case TokenType::NUMBER:
        os << node.number();
        break;
    case TokenType::BOOLEAN:
        os << std::boolalpha << node.boolean();
        break;
    case TokenType::NULL_VALUE:
        os << "null";
        break;
    default:
        break;
    }
}

inline std::ostream &operator<<(std::ostream &os, const JsonNode &node) {
    dump(node, os);
    return os;
}
} // namespace json
