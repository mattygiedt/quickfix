#pragma once

#include "quickfix/Application.h"
#include "quickfix/Message.h"
#include "quickfix/fix42/ExecutionReport.h"
#include "quickfix/fix42/MessageCracker.h"
#include "quickfix/fix42/NewOrderSingle.h"
#include "quickfix/fix42/OrderCancelReject.h"
#include "quickfix/fix42/OrderCancelReplaceRequest.h"
#include "quickfix/fix42/OrderCancelRequest.h"
#include "spdlog/spdlog.h"

// https://github.com/quickfix/quickfix/blob/master/examples/tradeclient/Application.cpp

namespace fixclient {

template <typename EventQueuePtr>
class Application : public FIX::Application, public FIX42::MessageCracker {
 private:
  const FIX::MsgType kExecutionReport{"8"};
  const FIX::MsgType kOrderCancelReject{"9"};

 public:
  Application(EventQueuePtr queue) : queue_(std::move(queue)) {
    queue_->appendListener(kExecutionReport, [&](const FIX::Message& msg,
                                                 const FIX::SessionID& sess) {
      spdlog::info("onExecutionReport: {}=>{}", sess.toString(),
                   msg.toString());
    });

    queue_->appendListener(kOrderCancelReject, [&](const FIX::Message& msg,
                                                   const FIX::SessionID& sess) {
      spdlog::info("onOrderCancelReject: {}=>{}", sess.toString(),
                   msg.toString());
    });
  }

  auto onCreate(const FIX::SessionID& session_id) -> void override {
    spdlog::info("session created: {}", session_id.toString());
  }

  auto onLogon(const FIX::SessionID& session_id) -> void override {
    spdlog::info("session logon: {}", session_id.toString());
  }

  auto onLogout(const FIX::SessionID& session_id) -> void override {
    spdlog::info("session logout: {}", session_id.toString());
  }

  auto toAdmin(FIX::Message& message, const FIX::SessionID&) -> void override {
    spdlog::info("toAdmin: {}", message.toString());
  }

  auto toApp(FIX::Message& message, const FIX::SessionID&)
      EXCEPT(FIX::DoNotSend) -> void override {
    spdlog::info("toApp: {}", message.toString());
  }

  auto fromAdmin(const FIX::Message& message, const FIX::SessionID&)
      EXCEPT(FIX::FieldNotFound, FIX::IncorrectDataFormat,
             FIX::IncorrectTagValue, FIX::RejectLogon) -> void override {
    spdlog::info("fromAdmin: {}", message.toString());
  }

  auto fromApp(const FIX::Message& message, const FIX::SessionID& sessionID)
      EXCEPT(FIX::FieldNotFound, FIX::IncorrectDataFormat,
             FIX::IncorrectTagValue, FIX::UnsupportedMessageType)
          -> void override {
    crack(message, sessionID);
  }

  auto onMessage(const FIX42::ExecutionReport& message,
                 const FIX::SessionID& sessionID) -> void override {
    FIX::MsgType msg_type;
    message.getHeader().get(msg_type);
    queue_->enqueue(msg_type, message, sessionID);
  }

  auto onMessage(const FIX42::OrderCancelReject& message,
                 const FIX::SessionID& sessionID) -> void override {
    FIX::MsgType msg_type;
    message.getHeader().get(msg_type);
    queue_->enqueue(msg_type, message, sessionID);
  }

 private:
  EventQueuePtr queue_;
};

}  // namespace fixclient
