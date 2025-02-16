#include "interpreted_program.hpp"
#include "server_core.hpp"

#include <networkprotocoldsl_uv/asyncworkqueue.hpp>

#include <iostream>
#include <span>
#include <string>
#include <thread>
#include <uv.h>
#include <vector>

int main(int argc, const char **argv) {
  std::cerr << "SMTP Server example started." << std::endl;

  // Retrieve the interpreted program from the embedded static source.
  auto maybe_program = smtpserver::load_interpreted_program();
  if (!maybe_program.has_value()) {
    std::cerr << "Failed to load the interpreted program." << std::endl;
    return 1;
  }

  // Set up libuv loop and async work queue.
  uv_loop_t *loop = uv_default_loop();
  networkprotocoldsl_uv::AsyncWorkQueue async_queue(loop);

  // Start the libuv loop in a separate thread.
  std::thread io_thread([&]() { uv_run(loop, UV_RUN_DEFAULT); });

  // Create a span from argv without creating a vector.
  std::span<const char *> args(argv, static_cast<size_t>(argc));
  smtpserver::main_server(args, *maybe_program, async_queue);

  async_queue.shutdown().wait();
  io_thread.join();

  return 0;
}