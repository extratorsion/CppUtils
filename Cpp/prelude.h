#ifndef PRELUD_H
#define PRELUD_H

#include <functional>
#include <memory>

#ifndef MACRO_LAN_EXT
#define MACRO_LAN_EXT

#define let const auto
#define mut auto
#define expr(block) [&] block ()
#define fn auto

#endif

template <typename T>
using Fn = std::function<T>;

template <typename T>
using Box = std::unique_ptr<T>;

template <typename T>
using Rc = std::shared_ptr<T>;


#endif