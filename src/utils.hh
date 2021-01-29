//
// Created by BlurryLight on 2021/1/27.
//

#pragma once
namespace PD {
class Noncopyable {
public:
  Noncopyable() = default;
  ~Noncopyable() = default;

private:
  Noncopyable(const Noncopyable &) = delete;
  Noncopyable &operator=(const Noncopyable &) = delete;
};

} // namespace PD
