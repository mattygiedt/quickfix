#include <iostream>
#include <string>
#include <thread>

#include "client_app.h"
#include "common/application_traits.h"

template <typename Traits>
class FixClient : public Traits {
 private:
  using ClientApplication =
      fixclient::Application<typename Traits::EventQueuePtr>;

 public:
  FixClient(std::string config)
      : config_(std::move(config)),
        queue_(std::make_shared<typename Traits::EventQueue>()),
        application_(queue_),
        initiator_{nullptr} {}

  auto Initialize() -> void {
    FIX::SessionSettings settings(config_);
    FIX::FileStoreFactory store_factory(settings);
    FIX::ScreenLogFactory log_factory(settings);

    initiator_ = std::make_unique<FIX::SocketInitiator>(
        application_, store_factory, settings, log_factory);
  }

  auto Start() -> void {
    initiator_->start();

    while (!initiator_->isStopped()) {
      if (queue_->emptyQueue()) {
        queue_->waitFor(Traits::kQueueWait);
      }

      queue_->process();
    }
  }

  auto Stop() -> void { initiator_->stop(); }

 private:
  std::string config_;
  typename Traits::EventQueuePtr queue_;
  ClientApplication application_;
  std::unique_ptr<FIX::Initiator> initiator_;
};

auto main(int argc, char** argv) -> int {
  if (argc < 2) {
    std::cout << "usage: " << argv[0] << " FILE." << std::endl;
    return 0;
  }

  std::string file = argv[1];
  spdlog::info("quickfix client config file: {}", file);

  FixClient<common::ClientTraits> client(file);
  client.Initialize();
  client.Start();
  client.Stop();
  return 1;
}
