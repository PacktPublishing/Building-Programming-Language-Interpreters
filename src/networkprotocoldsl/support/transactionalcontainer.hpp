#ifndef INCLUDED_NETWORKPROTOCOLDSL_SUPPORT_TRANSACTIONALCONTAINER_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SUPPORT_TRANSACTIONALCONTAINER_HPP

#include <atomic>
#include <memory>
#include <mutex>

namespace networkprotocoldsl::support {

template <typename Container> class TransactionalContainer {
  std::atomic<std::shared_ptr<const Container>> _container;

public:
  TransactionalContainer()
      : _container({std::make_shared<const Container>()}) {}
  TransactionalContainer(const TransactionalContainer &other) = delete;
  TransactionalContainer(TransactionalContainer &&other) = delete;

  template <typename Callable> void do_transaction(Callable func) {
    while (true) {
      auto current = _container.load();
      const std::shared_ptr<const Container> replacement = func(current);
      if (_container.compare_exchange_weak(current, replacement)) {
        break;
      }
    }
  }

  const std::shared_ptr<const Container> current() const {
    return _container.load();
  }
};

} // namespace networkprotocoldsl::support

#endif