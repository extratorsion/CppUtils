#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include "prelude.h"
#include <string_view>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>
#include <string>


namespace {

template<typename T>
inline fn __CatString(std::ostringstream& oss, T&& v) -> void {
    oss << std::forward<T>(v);
}

template<typename T, typename... Args>
inline fn __CatString(std::ostringstream& oss, T&& v, Args&&... args) -> void {
    oss << std::forward<T>(v);
    __CatString(oss, std::forward<Args>(args)...);
}

}

template <typename... Args>
fn FormatString(const char* fmt, Args... args) -> string {
    static thread_local char buffer[2048] = {0};
    ::memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), fmt, args...);
    return buffer;
}

template <size_t bufferSize, typename... Args>
fn FormatString(const char* fmt, Args... args) -> string {
    char buffer[bufferSize] = {0};
    snprintf(buffer, sizeof(buffer), fmt, args...);
    return buffer;
}

inline fn println() -> std::ostream& {
    std::cout << std::endl;;
    return std::cout;
}

template <typename T>
inline fn println(const T& val) -> std::ostream&  {
    std::cout << val << std::endl;
    return std::cout;
}

template <typename T, typename... Args>
fn println(const T& val, const Args&... args) -> std::ostream& {
    std::cout << val;
    return println(args...);
}


template<typename... Args>
fn CatString(Args&&... args) -> string {
    std::ostringstream oss;
    __CatString(oss, std::forward<Args>(args)...);
    return oss.str();
}

template <typename T=int>
fn SplitString(const string& content, char sep) -> vector<string> {
    // return: start_pos, finish_pos
    static auto strip = [](const char* str, size_t len) -> std::pair<const char*, const char*> {
        if (len <= 0) return {str, str};
        int start = 0, end = int(len) - 1;
        for (; start < int(len); ++start) {
            if (not(str[start] == ' ' || str[start] == '\t' || str[start] == '\n'))
                break;
        }
        for (; end >= 0; --end) {
            if (not(str[end] == ' ' || str[end] == '\t' || str[end] == '\n'))
                break;
        }
        return {str + start, str + end + 1};
    };

    vector<string> results;
    const char* cstr = content.c_str();
    size_t start = 0, finish = 0;
    for (size_t i = 0; i < content.size(); ++i) {
        if (sep == content[i] || i == content.size() - 1) {
            finish = (sep == content[i] ? i: content.size());
            if (start < finish) {
                auto [start_pos, finish_pos] = strip(cstr + start, finish - start);
                if (start_pos and finish_pos and start_pos < finish_pos) {
                    results.push_back(string(start_pos, finish_pos));
                }
            }
            start = i + 1;
        }
    }
    return results;
}

template <typename T=int>
fn JoinString(const vector<string>& strings, const string& sep) -> string {
    if (strings.empty()) return "";
    string builder;
    for (size_t i = 0; i < strings.size(); ++i) {
        builder.append(strings[i]);
        if (not(i == strings.size() - 1)) {
            builder.append(sep);
        }
    }
    return builder;
}

template <typename T=int>
fn CatString(const std::initializer_list<string>& strs) -> string {
    size_t totalSize = 0;
    std::for_each(strs.begin(), strs.end(), [&](const string& str){ totalSize += str.size(); });
    string builder;
    builder.reserve(totalSize + 1);
    for (auto& str: strs) {
        builder.append(str);
    }
    return builder;
}


#endif // STRINGUTILS_H
