#pragma once

#include <iostream>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>

namespace json {

/// JsonOut is used to print json objects. It is a simplified implementation
/// that does not create a real json object, but instead writes to the specified
/// output stream while pretending to create a object
///
/// Note that this class acts as a reference, and newly
/// created instances can be moved by value when copied to instead of being used
/// as references.
class JsonOut {
public:
    JsonOut &operator=(const JsonOut &) = delete;
    JsonOut &operator=(JsonOut &&) = delete;

    JsonOut(const JsonOut &) = delete;
    JsonOut(JsonOut &&j)
        : _os{std::exchange(j._os, nullptr)}
        , _parent{j._parent}
        , _indent{j._indent}
        , _is_object{j._is_object}
        , _is_array{j._is_array}
        , _has_pending_newline{j._has_pending_newline}
        , _indent_width{j._indent_width} {}

    explicit JsonOut(std::ostream &os,
                     int indent_width = 2,
                     int indent = 0,
                     JsonOut *parent = nullptr)
        : _os(&os)
        , _indent_width(indent_width)
        , _indent(indent)
        , _parent{parent} {}

    static inline JsonOut pretty(std::ostream &os) {
        return JsonOut{os};
    }

    /// Convenience for those who does not want to remember that -1 means no
    /// pretty print
    static inline JsonOut inlined(std::ostream &os) {
        return JsonOut{os, -1};
    }

    ~JsonOut() {
        finish();
    }

    void finish() {
        if (!_os) {
            return;
        }
        if (_is_object) {
            _indent -= 1;
            print_pending_newline(false);
            print_indent();
            *_os << "}";
        }
        if (_is_array) {
            _indent -= 1;
            print_pending_newline(false);
            print_indent();
            *_os << "]";
        }
        _os = nullptr;
    }

    template <typename T>
    JsonOut &operator=(const T &value) {
        try_colon();
        *_os << value;
        set_pending_newline();
        return *this;
    }

    JsonOut &operator=(const char *value) {
        return (*this) = std::string_view{value};
    }

    // This is to handle the idiotic standard in json where a list cannot end
    // with a comma
    void print_pending_newline(bool should_use_comma = true) {
        if (_has_pending_newline) {
            if (should_use_comma) {
                *_os << ",";
                new_line();
            }
            else {
                //                *_os << "\n";
                new_line();
            }
            _has_pending_newline = false;
        }
    }

    static void print_escaped(std::ostream &os, std::string_view value) {
        for (char c : value) {
            switch (c) {
            case '\"':
                os << "\\\"";
                break;
            case '\\':
                os << "\\\\";
                break;
            case '/':
                os << "\\/";
                break;
            case '\b':
                os << "\\b";
                break;
            case '\f':
                os << "\\f";
                break;
            case '\n':
                os << "\\n";
                break;
            case '\r':
                os << "\\r";
                break;
            case '\t':
                os << "\\t";
                break;
            default:
                os << c;
            }
        }
    }

    JsonOut &operator=(std::string_view value) {
        try_colon();
        *_os << "\"";
        print_escaped(*_os, value);
        *_os << "\"";
        set_pending_newline();
        return *this;
    }

    JsonOut &operator=(const std::string &value) {
        try_colon();
        *_os << "\"";
        print_escaped(*_os, value);
        *_os << "\"";
        set_pending_newline();
        return *this;
    }

    JsonOut operator[](std::string_view key) {
        print_pending_newline();
        if (!_is_object) {
            if (_parent && _parent->_is_object) {
                *_os << ": ";
            }
            *_os << "{";
            new_line();
            _is_object = true;
        }
        print_indent();
        *_os << "\"" << key << "\"";
        return JsonOut(*_os, _indent_width, _indent + 1, this);
    }

    JsonOut &operator=(std::nullptr_t) {
        try_colon();
        *_os << "null";
        set_pending_newline();
        return *this;
    }

    JsonOut &operator=(bool value) {
        try_colon();
        *_os << (value ? "true" : "false");
        set_pending_newline();
        return *this;
    }

    void set_pending_newline() {
        if (_parent) {
            _parent->_has_pending_newline = true;
        }
    }

    /// For arrays, create a new element on the back and
    JsonOut push_back() {
        print_pending_newline();
        if (!_is_array) {
            try_colon();
            *_os << "[";
            new_line();
        }
        print_indent();
        _is_array = true;
        _has_pending_newline = true;

        set_pending_newline();

        return JsonOut(*_os, _indent_width, _indent + 1, this);
    }

    /// More ideomatic way to do push_back. Assumes the current element is a
    /// array and push a element to it
    /// Note: To push objects: use push_back() (without arguments) and build a
    /// object on the result of that
    template <typename T>
    void push_back(const T &value) {
        push_back() = value;
    }

private:
    void try_colon() {
        if (_parent && _parent->_is_object) {
            *_os << ": ";
        }
    }

    std::ostream *_os = nullptr;
    JsonOut *_parent = nullptr;
    int _indent = 0;
    int8_t _indent_width = 0; // A value of -1 represents no indentation
    bool _is_object = false;
    bool _is_array = false;
    bool _has_pending_newline = false;

    void print_indent() {
        if (_indent_width == -1) {
            *_os << " ";
            return;
        }
        print_pending_newline();
        for (int i = 0; i < (_indent + 1) * _indent_width; ++i) {
            *_os << " ";
        }
    }

    void new_line() {
        if (_indent_width != -1) {
            *_os << "\n";
            return;
        }
    }
};

} // namespace json
