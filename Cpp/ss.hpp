#ifndef SS_HPP
#define SS_HPP

#include "prelude.h"
#include <list>

struct SS {
  using SlotSet = HashMap<void*, std::list<pair<void*, Fn<void*()>>>>;
  static SlotSet slots;

  template <typename Class, typename T, typename... Args>
  fn connect(Class* obj, void (Class::*p)(Args...), T&& val)->void {
    Fn<void(Args...)> raw_fun = std::move(val);
    Fn<void*()> fun = [_fun = raw_fun]() mutable {
      return reinterpret_cast<void*>(&_fun);
    };
    if (let iter = slots.find(static_cast<void*>(obj)); iter != slots.end()) {
      iter->second.push_back({pointer_cast(p), std::move(fun)});
    } else {
      std::list<pair<void*, Fn<void*()>>> l;
      l.push_back({pointer_cast(p), std::move(fun)});
      slots.insert({static_cast<void*>(obj), std::move(l)});
    }
  }

  template <typename Class, typename... Args>
  fn emit(Class* obj, void (Class::*p)(Args...), Args... args)->void {
    if (let iter = slots.find(static_cast<void*>(obj)); iter != slots.end()) {
      for (auto& [fun_sig, p_fun] : iter->second) {
        if (pointer_cast(p) == fun_sig) {
          auto fun = reinterpret_cast<Fn<void(Args...)>*>(p_fun());
          (*fun)(args...);
        }
      }
    }
  }

  template <typename Class, typename... Args>
  fn pointer_cast(void (Class::*p)(Args...))->void* {
    return (void*)(*reinterpret_cast<size_t*>(&p));
  }

  ~SS() {
    if (auto iter = slots.find(static_cast<void*>(this)); iter != slots.end()) {
      slots.erase(iter);
    }
  }
};


SS::SlotSet SS::slots;

#endif