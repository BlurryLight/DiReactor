//
// Created by BlurryLight on 2021/1/27.
//

#pragma once
#include "spdlog/spdlog.h"
#include <string>
namespace PD {

template <typename String, typename... Args>
inline void Log_Abort(const String &fmt, Args &&...args) {
  char buf[128];
  std::string fmtstring = std::string(fmt) + std::string("Because: {}");
  spdlog::error(fmtstring, std::forward<Args>(args)...,
                strerror_r(errno, buf, sizeof buf));
  std::abort();
}

template <typename T> inline T *CHECK_NOT_NULL(T *ptr) {
  if (!ptr)
    std::abort();
  return ptr;
}
class Noncopyable {
public:
  Noncopyable() = default;
  ~Noncopyable() = default;

private:
  Noncopyable(const Noncopyable &) = delete;
  Noncopyable &operator=(const Noncopyable &) = delete;
};

} // namespace PD
