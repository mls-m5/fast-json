#pragma once

#include "token.h"

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

private:
    Token _value;
    const JsonNode *_children = nullptr;
    const JsonNode *_next = nullptr;
};
