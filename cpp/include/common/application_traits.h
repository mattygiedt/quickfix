#pragma once

#include "common/time_util.h"
#include "eventpp/eventqueue.h"
#include "quickfix/Message.h"

namespace common {

struct ClientTraits {
  using EventQueue =
      eventpp::EventQueue<FIX::MsgType, void(const FIX::Message&)>;
};

struct ServerTraits {
  using EventQueue =
      eventpp::EventQueue<FIX::MsgType, void(const FIX::Message&)>;
};

}  // namespace common
