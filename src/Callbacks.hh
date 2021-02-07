//
// Created by BlurryLight on 2021/2/6.
//

#pragma once
#include <functional>
namespace PD {
using TimerCallbackFunc = std::function<void()>;
using EventCallbackFunc = std::function<void()>;
using Functor = std::function<void()>;
} // namespace PD
