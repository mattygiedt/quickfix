#pragma once

#include "common/time_util.h"
#include "quickfix/Application.h"
#include "quickfix/Message.h"
#include "quickfix/fix42/ExecutionReport.h"
#include "quickfix/fix42/MessageCracker.h"
#include "quickfix/fix42/NewOrderSingle.h"
#include "quickfix/fix42/OrderCancelReject.h"
#include "quickfix/fix42/OrderCancelReplaceRequest.h"
#include "quickfix/fix42/OrderCancelRequest.h"
#include "spdlog/spdlog.h"

namespace fixserver {

template <typename EventQueuePtr>
class Application : public FIX::Application, public FIX42::MessageCracker {
 private:
  using TimeUtil = common::TimeUtil;
  const FIX::MsgType kNewOrderSingle{"D"};
  const FIX::MsgType kOrderCancelRequest{"F"};

 public:
  Application(EventQueuePtr queue) : queue_(std::move(queue)) {
    queue_->appendListener(
        kNewOrderSingle,
        [&](const FIX::Message& message, const FIX::SessionID& sessionID) {
          spdlog::info("onNewOrderSingle: {}=>{}", sessionID.toString(),
                       message.toString());

          HandleNewOrderSingle(static_cast<FIX42::NewOrderSingle>(message),
                               sessionID);
        });

    queue_->appendListener(
        kOrderCancelRequest,
        [&](const FIX::Message& message, const FIX::SessionID& sessionID) {
          spdlog::info("onOrderCancelRequest: {}=>{}", sessionID.toString(),
                       message.toString());

          HandleOrderCancelRequest(
              static_cast<FIX42::OrderCancelRequest>(message), sessionID);
        });
  }

  auto GenerateId() -> std::string {
    return std::to_string(TimeUtil::EpochNanos());
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

  auto fromAdmin(const FIX::Message& message, const FIX::SessionID&)
      EXCEPT(FIX::FieldNotFound, FIX::IncorrectDataFormat,
             FIX::IncorrectTagValue, FIX::RejectLogon) -> void override {
    spdlog::info("fromAdmin: {}", message.toString());
  }

  auto toApp(FIX::Message& message, const FIX::SessionID&)
      EXCEPT(FIX::DoNotSend) -> void override {
    spdlog::info("toApp: {}", message.toString());
  }

  auto fromApp(const FIX::Message& message, const FIX::SessionID& sessionID)
      EXCEPT(FIX::FieldNotFound, FIX::IncorrectDataFormat,
             FIX::IncorrectTagValue, FIX::UnsupportedMessageType)
          -> void override {
    crack(message, sessionID);
  }

  auto onMessage(const FIX42::NewOrderSingle& message,
                 const FIX::SessionID& sessionID) -> void override {
    FIX::MsgType msg_type;
    message.getHeader().get(msg_type);
    queue_->enqueue(msg_type, message, sessionID);
  }

  auto onMessage(const FIX42::OrderCancelRequest& message,
                 const FIX::SessionID& sessionID) -> void override {
    FIX::MsgType msg_type;
    message.getHeader().get(msg_type);
    queue_->enqueue(msg_type, message, sessionID);
  }

  auto HandleNewOrderSingle(const FIX42::NewOrderSingle& message,
                            const FIX::SessionID& sessionID) -> void {
    FIX::Symbol symbol;
    FIX::Side side;
    FIX::OrdType ordType;
    FIX::OrderQty orderQty;
    FIX::Price price;
    FIX::ClOrdID clOrdID;
    FIX::Account account;

    message.get(ordType);

    if (ordType != FIX::OrdType_LIMIT) {
      throw FIX::IncorrectTagValue(ordType.getField());
    }

    message.get(symbol);
    message.get(side);
    message.get(orderQty);
    message.get(price);
    message.get(clOrdID);

    auto orderID = GenerateId();
    auto execID = GenerateId();

    FIX42::ExecutionReport executionReport(
        FIX::OrderID(orderID), FIX::ExecID(execID),
        FIX::ExecTransType(FIX::ExecTransType_NEW),
        FIX::ExecType(FIX::ExecType_FILL),
        FIX::OrdStatus(FIX::OrdStatus_FILLED), symbol, side, FIX::LeavesQty(0),
        FIX::CumQty(orderQty), FIX::AvgPx(price));

    executionReport.set(clOrdID);
    executionReport.set(orderQty);
    executionReport.set(FIX::LastShares(orderQty));
    executionReport.set(FIX::LastPx(price));

    FIX::Session::sendToTarget(executionReport, sessionID);
  }

  auto HandleOrderCancelRequest(const FIX42::OrderCancelRequest& message,
                                const FIX::SessionID& sessionID) -> void {
    FIX::OrigClOrdID origClOrdID;
    FIX::ClOrdID clOrdID;
    FIX::OrderID orderID;
    FIX::Symbol symbol;
    FIX::Side side;

    message.get(origClOrdID);
    message.get(clOrdID);
    message.get(orderID);
    message.get(symbol);
    message.get(side);

    FIX42::OrderCancelReject orderCancelReject(
        orderID, clOrdID, origClOrdID,
        FIX::OrdStatus(FIX::OrdStatus_DONE_FOR_DAY),
        FIX::CxlRejResponseTo('1'));

    FIX::Session::sendToTarget(orderCancelReject, sessionID);
  }

 private:
  EventQueuePtr queue_;
};

}  // namespace fixserver
