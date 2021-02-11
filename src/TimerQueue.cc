//
// Created by BlurryLight on 2021/2/6.
//

#include "TimerQueue.hh"
#include "EventLoop.hh"
#include "Timer.hh"
#include "TimerProxy.hh"
#include "spdlog/spdlog.h"
#include <sys/timerfd.h>
#include <unistd.h>

static int create_timerfd() {
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  char buf[128];
  if (timerfd < 0) {
    spdlog::error("ERROR:timerfd_create failed: {}",
                  strerror_r(errno, buf, 128));
  }
  return timerfd;
}

static void read_timerfd(int timerfd) {
  uint64_t tmp;
  ssize_t n = ::read(timerfd, &tmp, sizeof tmp);
  spdlog::info("TimerQueue::handle_timerfd() {}  at  {}", tmp,
               std::chrono::system_clock::now().time_since_epoch().count());
  if (n != sizeof(tmp)) {
    spdlog::error("TimerQueue::handle_timerfd read {} bytes!", n);
  }
}
static struct timespec time_from_now(PD::time_point when) {
  auto duration = when - std::chrono::system_clock::now();
  auto secs = std::chrono::duration_cast<std::chrono::seconds>(duration);
  auto nanosecs =
      std::chrono::duration_cast<std::chrono::nanoseconds>(duration - secs);
  return timespec{secs.count(), nanosecs.count()};
}

static void reset_timerfd(int timerfd, PD::time_point expiration) {
  struct itimerspec new_ts, old_ts;
  bzero(&new_ts, sizeof new_ts);
  bzero(&old_ts, sizeof old_ts);
  new_ts.it_value = time_from_now(expiration);
  assert(new_ts.it_value.tv_nsec >= 0);
  assert(new_ts.it_value.tv_sec >= 0);
  int ret = ::timerfd_settime(timerfd, 0, &new_ts, &old_ts);
  if (ret) {
    char buf[128];
    spdlog::error("timerfd_settime(),error {} ", strerror_r(errno, buf, 128));
    spdlog::info("new_ts {} {}", new_ts.it_value.tv_sec,
                 new_ts.it_value.tv_nsec);
  }
}
using namespace PD;
TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop), timerfd_(create_timerfd()), timerfdChannel_(loop, timerfd_),
      timers_() {
  timerfdChannel_.set_read_callback(
      std::bind(&TimerQueue::handle_timerfd, this));
  timerfdChannel_.enable_reading();
}

TimerQueue::~TimerQueue() {
  // RAII
  ::close(timerfd_);
}
void TimerQueue::handle_timerfd() {
  loop_->assert_in_thread();
  auto now = std::chrono::system_clock::now();
  read_timerfd(timerfd_);

  std::vector<Entry> expired = getExpired(now);
  for (const auto &it : expired) {
    it.second->run();
  }
  reset(expired, now);
}
std::vector<TimerQueue::Entry> TimerQueue::getExpired(time_point now) {
  std::vector<Entry> expired;
  Entry sentry = std::make_pair(now, std::unique_ptr<Timer>(nullptr));
  auto it = timers_.lower_bound(sentry);
  assert(it == timers_.end() || now < it->first);
  for (auto itbegin = timers_.begin(); itbegin != it;) {
    // only in C++17 with the help of std::set::extract when it is possible to
    // transfer unique_ptr from a set to vector
    auto next_it = std::next(itbegin);
    auto pair = timers_.extract(itbegin);
    expired.push_back(std::move(pair.value()));
    itbegin = next_it;
  }
  return expired;
}
void TimerQueue::reset(std::vector<Entry> &expired, time_point now) {
  time_point next_expire;
  for (auto &it : expired) {
    if (it.second->repeat()) {
      it.second->restart(now);
      insert(std::move(it.second));
    }
  }
  if (!timers_.empty()) {
    next_expire = timers_.begin()->second->expiration();
    reset_timerfd(timerfd_, next_expire);
  }
}
bool TimerQueue::insert(std::unique_ptr<Timer> timer) {
  bool earliestChanged = false;
  auto when = timer->expiration();
  auto it = timers_.begin();
  if (it == timers_.end() || when < it->first) {
    earliestChanged = true;
  }
  auto res = timers_.insert(std::make_pair(when, std::move(timer)));
  assert(res.second);
  return earliestChanged;
}
bool TimerQueue::insert(Timer *timer) {
  return insert(std::unique_ptr<Timer>(timer));
}
TimerProxy TimerQueue::addTimer(const TimerCallbackFunc &cb,
                                time_point expiration, double intervalS) {

  //这里不能用unique_ptr
  //因为ptr的生命周期至少应该延长到addTimerInLoop实际发生
  //也就是可能到下一次事件循环
  auto ptr = new Timer(cb, expiration, intervalS);
  loop_->run_in_loop([ptr, this]() { addTimerInLoop(ptr); });
  return TimerProxy(ptr);
}
void TimerQueue::addTimerInLoop(Timer *timer) {

  loop_->assert_in_thread();
  auto expiration = timer->expiration();
  bool earliestChanged = insert(timer);
  if (earliestChanged) {
    reset_timerfd(timerfd_, expiration);
  }
}
