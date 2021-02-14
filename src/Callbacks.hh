//
// Created by BlurryLight on 2021/2/6.
//

#pragma once
#include <functional>
#include <memory>
namespace PD {
using TimerCallbackFunc = std::function<void()>;
using EventCallbackFunc = std::function<void()>;
using Functor = std::function<void()>;

class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using MessageCallback = std::function<void(const TcpConnectionPtr &,
                                           const char *data, ssize_t len)>;
} // namespace PD
