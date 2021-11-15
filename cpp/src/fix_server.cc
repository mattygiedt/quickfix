#include <iostream>
#include <string>
#include <thread>

#include "common/application_traits.h"
#include "server_app.h"

std::atomic<bool> running_;
std::mutex running_mutex_;
std::condition_variable running_cv_;

auto SignalHandler(int /*unused*/) -> void {
  running_ = false;
  running_cv_.notify_all();
}

auto SetupSignalHandler() -> void {
  struct sigaction sig_int_handler;
  sig_int_handler.sa_handler = SignalHandler;
  sigemptyset(&sig_int_handler.sa_mask);
  sig_int_handler.sa_flags = 0;
  sigaction(SIGINT, &sig_int_handler, nullptr);
}

auto WaitForSignal() -> void {
  running_ = true;
  while (running_) {
    std::unique_lock<decltype(running_mutex_)> lock(running_mutex_);
    running_cv_.wait(lock);
  }
}

template <typename Traits>
class FixServer {
 private:
  using ServerApplication =
      fixserver::Application<typename Traits::EventQueuePtr>;

 public:
  FixServer(std::string config)
      : config_(std::move(config)),
        queue_(std::make_shared<typename Traits::EventQueue>()),
        application_(queue_),
        acceptor_{nullptr} {}

  auto Initialize() -> void {
    FIX::SessionSettings settings(config_);
    FIX::FileStoreFactory store_factory(settings);
    FIX::ScreenLogFactory log_factory(settings);

    acceptor_ = std::make_unique<FIX::SocketAcceptor>(
        application_, store_factory, settings, log_factory);
  }

  auto Start() -> void {
    acceptor_->start();
    process_thread_ = std::thread([&]() {
      while (!acceptor_->isStopped()) {
        if (queue_->emptyQueue()) {
          queue_->waitFor(Traits::kQueueWait);
        }

        queue_->process();
      }
    });
  }

  auto Stop() -> void {
    acceptor_->stop();
    process_thread_.join();
  }

 private:
  std::string config_;
  typename Traits::EventQueuePtr queue_;
  ServerApplication application_;
  std::unique_ptr<FIX::Acceptor> acceptor_;
  std::thread process_thread_;
};

auto main(int argc, char** argv) -> int {
  if (argc < 2) {
    std::cout << "usage: " << argv[0] << " FILE." << std::endl;
    return 0;
  }

  std::string file = argv[1];
  spdlog::info("quickfix server config file: {}", file);

  FixServer<common::ServerTraits> server(file);
  server.Initialize();
  server.Start();
  WaitForSignal();
  server.Stop();
  return 1;
}
