# quickfix
Generic QuickFIX and eventpp examples

## Building
Project uses VSCode `.devcontainer` support -- look in the directory for a sample `Dockerfile` if you want to roll your own.

## Workflow
After a successful login, client sends the server a `NewOrderSingle` which gets fully executed, and the `ExecutionReport` is sent back to the client. Client then tries to `OrderCancelRequest` the order, which is handled with a `OrderCancelReject`.

## eventpp
An `eventpp::EventQueue` from [eventpp](https://github.com/wqking/eventpp) is used to handle the cracked message, decoupling the FIX workflow from business logic.

## Simple but powerful
While this is a trivial example, the client / server framework can be immediately extended by swapping out the `Application` class to fit your needs.
```
  // replace me...
  using ClientApplication =
      fixclient::Application<typename Traits::EventQueuePtr>;
      
  // replace me...
  using ServerApplication =
      fixserver::Application<typename Traits::EventQueuePtr>;
```
