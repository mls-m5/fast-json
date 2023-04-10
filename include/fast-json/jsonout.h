#include <iostream>
#include <string>

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
        _os << ": \"" << value << "\",\n";
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
