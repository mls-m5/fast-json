#pragma once

#include <iostream>
#include <ostream>
#include <string>
#include <string_view>

namespace json {

class JsonOut {
public:
    explicit JsonOut(std::ostream &os, int indent = 0)
        : _os(os)
        , _indent(indent) {}

    ~JsonOut() {
        if (_is_object) {
            _indent -= 1;
            print_indent();
            _os << "}\n";
        }
    }

    template <typename T>
    JsonOut &operator=(const T &value) {
        _os << ": " << value << ",\n";
        return *this;
    }

    JsonOut &operator=(const char *value) {
        //        _os << ": \"" << std::string_view{value} << "\",\n";
        return (*this) = std::string_view{value};
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
        _os << ": \"";
        print_escaped(_os, value);
        _os << "\",\n";
        return *this;
    }

    JsonOut &operator=(const std::string &value) {
        _os << ": \"";
        print_escaped(_os, value);
        _os << "\",\n";
        return *this;
    }

    JsonOut operator[](std::string_view key) {
        if (!_is_object) {
            if (_indent > 0) {
                _os << ": ";
            }
            _os << "{\n";
            _is_object = true;
        }
        print_indent();
        _os << "\"" << key << "\"";
        return JsonOut(_os, _indent + 1);
    }

    JsonOut &operator=(std::nullptr_t) {
        _os << ": null,\n";
        return *this;
    }

private:
    std::ostream &_os;
    int _indent = 0;
    bool _is_object = false;

    void print_indent() {
        for (int i = 0; i < _indent + 1; ++i) {
            _os << "  ";
        }
    }
};

} // namespace json
