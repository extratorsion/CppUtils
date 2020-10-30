#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string_view>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>
#include <string>

namespace sauron::utils {

namespace {

template<typename T>
inline void __CatString(std::ostringstream& oss, T&& v) {
    oss << std::forward<T>(v);
}

template<typename T, typename... Args>
inline void __CatString(std::ostringstream& oss, T&& v, Args&&... args) {
    oss << std::forward<T>(v);
    __CatString(oss, std::forward<Args>(args)...);
}

}

template <typename... Args>
std::string FormatString(const char* fmt, Args... args) {
    static thread_local char buffer[2048] = {0};
    ::memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), fmt, args...);
    return buffer;
}

template <size_t bufferSize, typename... Args>
std::string FormatString(const char* fmt, Args... args) {
    char buffer[bufferSize] = {0};
    snprintf(buffer, sizeof(buffer), fmt, args...);
    return buffer;
}

std::ostream& println();

template <typename T>
std::ostream& println(const T& val) {
    std::cout << val << std::endl;;
    return std::cout;
}

template <typename T, typename... Args>
std::ostream& println(const T& val, const Args&... args) {
    std::cout << val;
    return println(args...);
}


std::vector<std::string> SplitString(const std::string& content, char sep);

std::string JoinString(const std::vector<std::string>& strings, const std::string& sep);

std::string CatString(const std::initializer_list<std::string>& strs);

template<typename... Args>
std::string CatString(Args&&... args) {
    std::ostringstream oss;
    __CatString(oss, std::forward<Args>(args)...);
    return oss.str();
}

#include "utils/stringutils.h"

namespace sauron::utils {

std::vector<std::string> SplitString(const std::string& content, char sep) {
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

    std::vector<std::string> results;
    const char* cstr = content.c_str();
    size_t start = 0, finish = 0;
    for (size_t i = 0; i < content.size(); ++i) {
        if (sep == content[i] || i == content.size() - 1) {
            finish = (sep == content[i] ? i: content.size());
            if (start < finish) {
                auto [start_pos, finish_pos] = strip(cstr + start, finish - start);
                if (start_pos and finish_pos and start_pos < finish_pos) {
                    results.push_back(std::string(start_pos, finish_pos));
                }
            }
            start = i + 1;
        }
    }
    return results;
}

std::string JoinString(const std::vector<std::string>& strings, const std::string& sep) {
    if (strings.empty()) return "";
    std::string builder;
    for (size_t i = 0; i < strings.size(); ++i) {
        builder.append(strings[i]);
        if (not(i == strings.size() - 1)) {
            builder.append(sep);
        }
    }
    return builder;
}

std::string CatString(const std::initializer_list<std::string>& strs) {
    size_t totalSize = 0;
    std::for_each(strs.begin(), strs.end(), [&](const std::string& str){ totalSize += str.size(); });
    std::string builder;
    builder.reserve(totalSize + 1);
    for (auto& str: strs) {
        builder.append(str);
    }
    return builder;
}

std::ostream &println()
{
    std::cout << std::endl;;
    return std::cout;
}

}

}

#endif // STRINGUTILS_H
