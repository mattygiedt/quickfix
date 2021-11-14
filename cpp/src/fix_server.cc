#include <iostream>
#include <string>

#include "common/application_traits.h"
#include "server_app.h"

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
    while (!acceptor_->isStopped()) {
      if (queue_->emptyQueue()) {
        queue_->waitFor(Traits::kQueueWait);
      }

      queue_->process();
    }
  }

  auto Stop() -> void { acceptor_->stop(); }

 private:
  std::string config_;
  typename Traits::EventQueuePtr queue_;
  ServerApplication application_;
  std::unique_ptr<FIX::Acceptor> acceptor_;
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
  server.Stop();
  return 1;
}
