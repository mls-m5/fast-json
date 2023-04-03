#pragma once

#include "token.h"
#include <stdexcept>
#include <string>
#include <string_view>

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

    const_iterator begin() const {
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

    const JsonNode &operator*() const {
        validate_key();
        return *_children;
    }

    const JsonNode *operator->() const {
        validate_key();
        return _children;
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

    Token _value;
    const JsonNode *_children = nullptr;
    const JsonNode *_next = nullptr;
};
