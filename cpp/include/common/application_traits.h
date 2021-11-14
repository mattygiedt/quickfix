#pragma once

#include <condition_variable>
#include <csignal>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>

#include "common/time_util.h"
#include "eventpp/eventqueue.h"
#include "quickfix/FileLog.h"
#include "quickfix/FileStore.h"
#include "quickfix/Message.h"
#include "quickfix/SessionSettings.h"
#include "quickfix/SocketAcceptor.h"
#include "quickfix/SocketInitiator.h"
#include "quickfix/config.h"
#include "spdlog/spdlog.h"

namespace common {

struct CommonTraits {
  using EventQueue =
      eventpp::EventQueue<FIX::MsgType,
                          void(const FIX::Message&, const FIX::SessionID&)>;
  using EventQueuePtr = std::shared_ptr<EventQueue>;
};

struct ClientTraits : public CommonTraits {
  static constexpr auto kQueueWait = std::chrono::milliseconds(100);

  static auto GetSessionID() -> FIX::SessionID {
    return FIX::SessionID("FIX.4.2", "FIXCLIENT", "FIXSERVER");
  }
};

struct ServerTraits : public CommonTraits {
  static constexpr auto kQueueWait = std::chrono::milliseconds(100);

  static auto GetSessionID() -> FIX::SessionID {
    return FIX::SessionID("FIX.4.2", "FIXSERVER", "FIXCLIENT");
  }
};

}  // namespace common
