#ifndef INCLUDED_NETWORKPROTOCOLDSL_SUPPORT_NOTIFICATIONSIGNAL_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SUPPORT_NOTIFICATIONSIGNAL_HPP

#include <condition_variable>
#include <iostream>
#include <mutex>

#include <thread>

#define NOTIFICATIONSIGNAL_DEBUG(x)
//#define NOTIFICATIONSIGNAL_DEBUG(x) std::cerr << "NotificationSignal" << "["
//<< name << "/" << std::this_thread::get_id() << "] " << __func__ << ": " << x
//<< std::endl

namespace networkprotocoldsl::support {

class NotificationSignal {
  std::string name;
  std::mutex mtx;
  std::condition_variable cv;
  std::atomic<bool> notified = false;

public:
  NotificationSignal(const std::string &n)
      : name(n), mtx(std::mutex()), cv(std::condition_variable()) {}
  NotificationSignal(const NotificationSignal &in) = delete;
  NotificationSignal(NotificationSignal &&in) = delete;
  NotificationSignal &operator=(const NotificationSignal &) = delete;

  void notify() {
    {
      NOTIFICATIONSIGNAL_DEBUG("before guard");
      std::lock_guard<std::mutex> lk(mtx);
      NOTIFICATIONSIGNAL_DEBUG("after guard");
      notified.store(true);
    }
    NOTIFICATIONSIGNAL_DEBUG("before notify");
    cv.notify_all();
    NOTIFICATIONSIGNAL_DEBUG("after notify");
  }

  void wait() {
    NOTIFICATIONSIGNAL_DEBUG("before lock");
    std::unique_lock<std::mutex> lk(mtx);
    NOTIFICATIONSIGNAL_DEBUG("after lock");
    if (!notified.load()) {
      NOTIFICATIONSIGNAL_DEBUG("not notified, before wait");
      cv.wait(lk);
      NOTIFICATIONSIGNAL_DEBUG("not notified, after wait");
    } else {
      NOTIFICATIONSIGNAL_DEBUG("no wait");
    }
    notified.store(false);
  }
};

} // namespace networkprotocoldsl::support

#endif
