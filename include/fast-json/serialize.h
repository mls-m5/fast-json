#pragma once

#if 0
#define FAST_JSON_SERIALIZE_INTRUSIVE2(x, y)                                   \
    void to_json(json::JsonOut &json) const {                                  \
        json[#x] = x;                                                          \
        json[#y] = y;                                                          \
    }                                                                          \
    void from_json(const json::JsonNode &json) {                               \
        json[#x].get_to(x);                                                    \
        json[#y].get_to(y);                                                    \
    }

#endif

// Helper macros to count number of arguments
#define VA_NUM_ARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define VA_NUM_ARGS(...)                                                       \
    VA_NUM_ARGS_IMPL(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

// Helper macro to concatenate tokens
#define CONCATENATE_IMPL(s1, s2) s1##s2
#define CONCATENATE(s1, s2) CONCATENATE_IMPL(s1, s2)

#define _FAST_JSON_SERIALIZE_INTRUSIVE_0()                                     \
    void to_json(json::JsonOut &json) const {}                                 \
    void from_json(const json::JsonNode &json) {}

#define _FAST_JSON_SERIALIZE_INTRUSIVE_1(X0)                                   \
    void to_json(json::JsonOut &json) const {                                  \
        json[#X0] = X0;                                                        \
    }                                                                          \
    void from_json(const json::JsonNode &json) {                               \
        json[#X0].get_to(X0);                                                  \
    }

#define _FAST_JSON_SERIALIZE_INTRUSIVE_2(X0, X1)                               \
    void to_json(json::JsonOut &json) const {                                  \
        json[#X0] = X0;                                                        \
        json[#X1] = X1;                                                        \
    }                                                                          \
    void from_json(const json::JsonNode &json) {                               \
        json[#X0].get_to(X0);                                                  \
        json[#X1].get_to(X1);                                                  \
    }

#define _FAST_JSON_SERIALIZE_INTRUSIVE_3(X0, X1, X2)                           \
    void to_json(json::JsonOut &json) const {                                  \
        json[#X0] = X0;                                                        \
        json[#X1] = X1;                                                        \
        json[#X2] = X2;                                                        \
    }                                                                          \
    void from_json(const json::JsonNode &json) {                               \
        json[#X0].get_to(X0);                                                  \
        json[#X1].get_to(X1);                                                  \
        json[#X2].get_to(X2);                                                  \
    }

#define _FAST_JSON_SERIALIZE_INTRUSIVE_4(X0, X1, X2, X3)                       \
    void to_json(json::JsonOut &json) const {                                  \
        json[#X0] = X0;                                                        \
        json[#X1] = X1;                                                        \
        json[#X2] = X2;                                                        \
        json[#X3] = X3;                                                        \
    }                                                                          \
    void from_json(const json::JsonNode &json) {                               \
        json[#X0].get_to(X0);                                                  \
        json[#X1].get_to(X1);                                                  \
        json[#X2].get_to(X2);                                                  \
        json[#X3].get_to(X3);                                                  \
    }

#define _FAST_JSON_SERIALIZE_INTRUSIVE_5(X0, X1, X2, X3, X4)                   \
    void to_json(json::JsonOut &json) const {                                  \
        json[#X0] = X0;                                                        \
        json[#X1] = X1;                                                        \
        json[#X2] = X2;                                                        \
        json[#X3] = X3;                                                        \
        json[#X4] = X4;                                                        \
    }                                                                          \
    void from_json(const json::JsonNode &json) {                               \
        json[#X0].get_to(X0);                                                  \
        json[#X1].get_to(X1);                                                  \
        json[#X2].get_to(X2);                                                  \
        json[#X3].get_to(X3);                                                  \
        json[#X4].get_to(X4);                                                  \
    }
// Add more _FAST_JSON_SERIALIZE_INTRUSIVE_N for N=4,6,8,10,...

// Main macro that selects the appropriate implementation based on argument
// count
#define FAST_JSON_SERIALIZE_INTRUSIVE(...)                                     \
    CONCATENATE(_FAST_JSON_SERIALIZE_INTRUSIVE_, VA_NUM_ARGS(__VA_ARGS__))     \
    (__VA_ARGS__)
