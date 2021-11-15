#pragma once

#include "quickfix/Application.h"
#include "quickfix/Message.h"
#include "quickfix/Session.h"
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
    queue_->appendListener(
        kExecutionReport,
        [&](const FIX::Message& message, const FIX::SessionID& sessionID) {
          spdlog::info("onExecutionReport: {}=>{}", sessionID.toString(),
                       message.toString());

          HandleExecutionReport(static_cast<FIX42::ExecutionReport>(message),
                                sessionID);
        });

    queue_->appendListener(
        kOrderCancelReject,
        [&](const FIX::Message& message, const FIX::SessionID& sessionID) {
          spdlog::info("onOrderCancelReject: {}=>{}", sessionID.toString(),
                       message.toString());
          HandleOrderCancelReject(
              static_cast<FIX42::OrderCancelReject>(message), sessionID);
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

  auto SendNewOrderSingle(const std::string& client_order_id,
                          const FIX::SessionID& sessionID)
      -> FIX42::NewOrderSingle {
    FIX::OrdType ordType;

    FIX42::NewOrderSingle newOrderSingle(
        FIX::ClOrdID(client_order_id), FIX::HandlInst('1'), FIX::Symbol("ESZ1"),
        FIX::Side(FIX::Side_BUY), FIX::TransactTime(),
        FIX::OrdType(FIX::OrdType_LIMIT));

    newOrderSingle.setField(FIX::OrderQty(33));           // NOLINT
    newOrderSingle.setField(FIX::Price(1912));            // NOLINT
    newOrderSingle.setField(FIX::SecurityID("123456"));   // NOLINT
    newOrderSingle.setField(FIX::SecurityIDSource("8"));  // Exchange Symbol
    newOrderSingle.setField(FIX::TimeInForce(FIX::TimeInForce_DAY));

    FIX::Session::sendToTarget(newOrderSingle, sessionID);

    return newOrderSingle;
  }

  auto HandleExecutionReport(const FIX42::ExecutionReport& message,
                             const FIX::SessionID& sessionID) -> void {
    FIX::OrderQty orderQty;
    FIX::OrigClOrdID origClOrdID;
    FIX::ClOrdID clOrdID;
    FIX::OrderID orderID;
    FIX::Symbol symbol;
    FIX::Side side;

    message.get(clOrdID);
    message.get(orderID);
    message.get(orderQty);
    message.get(symbol);
    message.get(side);
    origClOrdID.setValue(clOrdID.getValue());

    FIX42::OrderCancelRequest orderCancelRequest(origClOrdID, clOrdID, symbol,
                                                 side, FIX::TransactTime());

    orderCancelRequest.set(orderID);
    orderCancelRequest.set(orderQty);

    FIX::Session::sendToTarget(orderCancelRequest, sessionID);
  }

  auto HandleOrderCancelReject(const FIX42::OrderCancelReject& /*unused*/,
                               const FIX::SessionID& /*unused*/) -> void {}

 private:
  EventQueuePtr queue_;
};

}  // namespace fixclient
