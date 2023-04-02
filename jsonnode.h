#pragma once

#include "token.h"

class JsonNode {
public:
    JsonNode(const Token& value) : _value(value) {}

    void children(JsonNode* children, size_t numChildren) {
        _children = children;
        _numChildren = numChildren;
    }

    const Token& value() const {
        return _value;
    }

    const JsonNode* children() const {
        return _children;
    }

    void children(const JsonNode* children) {
        _children = children;
    }

    size_t numChildren() const {
        return _numChildren;
    }

private:
    Token _value;
    JsonNode* _children = nullptr;
    size_t _numChildren = 0;
};