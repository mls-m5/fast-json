#pragma once

#include <iostream>
#include <ostream>
#include <string>
#include <string_view>

namespace json {

class JsonOut {
public:
    JsonOut(const JsonOut &) = default;
    JsonOut(JsonOut &&) = default;
    JsonOut &operator=(const JsonOut &) = default;
    JsonOut &operator=(JsonOut &&) = default;

    explicit JsonOut(std::ostream &os,
                     int indent = 0,
                     JsonOut *parent = nullptr)
        : _os(&os)
        , _indent(indent)
        , _parent{parent} {}

    ~JsonOut() {
        if (_is_object) {
            _indent -= 1;
            print_pending_newline(false);
            print_indent();
            *_os << "}\n";
        }
    }

    template <typename T>
    JsonOut &operator=(const T &value) {
        *_os << ": " << value; // << ",\n";
        set_pending_newline();
        return *this;
    }

    JsonOut &operator=(const char *value) {
        //        _os << ": \"" << std::string_view{value} << "\",\n";
        return (*this) = std::string_view{value};
    }

    // This is to handle the idiotic standard in json where a list cannot end
    // with a comma
    void print_pending_newline(bool should_use_comma = true) {
        if (_has_pending_newline) {
            if (should_use_comma) {
                *_os << ",\n";
            }
            else {
                *_os << "\n";
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
        *_os << ": \"";
        print_escaped(*_os, value);
        *_os << "\"";
        set_pending_newline();
        return *this;
    }

    JsonOut &operator=(const std::string &value) {
        *_os << ": \"";
        print_escaped(*_os, value);
        *_os << "\"";
        set_pending_newline();
        return *this;
    }

    JsonOut operator[](std::string_view key) {
        print_pending_newline();
        if (!_is_object) {
            if (_indent > 0) {
                *_os << ": ";
            }
            *_os << "{\n";
            _is_object = true;
        }
        print_indent();
        *_os << "\"" << key << "\"";
        return JsonOut(*_os, _indent + 1, this);
    }

    JsonOut &operator=(std::nullptr_t) {
        *_os << ": null";
        set_pending_newline();
        return *this;
    }

    JsonOut &operator=(bool value) {
        *_os << ": " << (value ? "true" : "false");
        set_pending_newline();
        return *this;
    }

    void set_pending_newline() {
        if (_parent) {
            _parent->_has_pending_newline = true;
        }
    }

private:
    std::ostream *_os = nullptr;
    JsonOut *_parent = nullptr;
    int _indent = 0;
    bool _is_object = false;
    bool _has_pending_newline = false;

    void print_indent() {
        print_pending_newline();
        for (int i = 0; i < _indent + 1; ++i) {
            *_os << "  ";
        }
    }
};

} // namespace json
