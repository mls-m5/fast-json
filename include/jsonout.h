#include <iostream>
#include <string>

class JsonOut {
public:
    explicit JsonOut(std::ostream &os, int indent = 0)
        : _os(os)
        , _indent(indent)
        , _is_root(indent == 0) {
        if (_is_root) {
            _os << "{\n";
        }
    }

    ~JsonOut() {
        if (_is_root) {
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

    JsonOut operator[](const std::string &key) {
        print_indent();
        _os << "\"" << key << "\"";
        return JsonOut(_os, _indent + 1);
    }

private:
    std::ostream &_os;
    int _indent;
    bool _is_root;

    void print_indent() {
        for (int i = 0; i < _indent; ++i) {
            _os << "  ";
        }
    }
};
