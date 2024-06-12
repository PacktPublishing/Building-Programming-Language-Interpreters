#ifndef INCLUDED_NETWORKPROTOCOLDSL_SUPPORT_NOTIFICATIONSIGNAL_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SUPPORT_NOTIFICATIONSIGNAL_HPP

#include <condition_variable>
#include <iostream>
#include <mutex>

namespace networkprotocoldsl::support {

class NotificationSignal {
  std::mutex mtx;
  std::condition_variable cv;
  bool notified = false;

public:
  NotificationSignal() : mtx(std::mutex()), cv(std::condition_variable()) {}
  NotificationSignal(const NotificationSignal &in) = delete;
  NotificationSignal(NotificationSignal &&in) = delete;
  NotificationSignal &operator=(const NotificationSignal &) = delete;

  void notify() {
    {
      std::lock_guard<std::mutex> lk(mtx);
      notified = true;
    }
    cv.notify_all();
  }

  void wait() {
    std::unique_lock<std::mutex> lk(mtx);
    if (!notified) {
      cv.wait(lk);
    }
    notified = false;
  }
};

} // namespace networkprotocoldsl::support

#endif
