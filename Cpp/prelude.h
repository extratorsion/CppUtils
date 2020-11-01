#ifndef PRELUD_H
#define PRELUD_H

#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#ifndef MACRO_LAN_EXT
#define MACRO_LAN_EXT

#define let const auto
#define var auto
#define expr(block) [&] block ()
#define fn auto

#endif

template <typename T>
using Fn = std::function<T>;

template <typename T>
using Box = std::unique_ptr<T>;

template <typename T>
using Rc = std::shared_ptr<T>;


using std::string;
using std::tuple;
using std::vector;

using std::make_unique;
using std::make_shared;
using std::make_tuple;

using std::move;
using std::forward;
using std::forward_as_tuple;

#endif