#pragma once

#include "token.h"

class JsonNode {
public:
    JsonNode(const JsonNode &) = delete;
    JsonNode(JsonNode &&) = default;
    JsonNode &operator=(const JsonNode &) = delete;
    JsonNode &operator=(JsonNode &&) = default;
    JsonNode() = default;
    JsonNode(const Token &value)
        : _value(value) {}

    void children(JsonNode *children, size_t numChildren) {
        _children = children;
        _numChildren = numChildren;
    }

    const Token &value() const {
        return _value;
    }

    const JsonNode *children() const {
        return _children;
    }

    void children(const JsonNode *children) {
        _children = children;
    }

    size_t numChildren() const {
        return _numChildren;
    }

    size_t numChildren(size_t num) {
        return _numChildren = num;
    }

private:
    Token _value;
    const JsonNode *_children = nullptr;
    size_t _numChildren = 0;
};
