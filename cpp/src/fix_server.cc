#include <iostream>
#include <string>

#include "common/application_traits.h"

template <typename Traits>
class FixServer {
 private:
 public:
 private:
  typename Traits::EventQueue queue_;
};

auto main(int argc, char** argv) -> int {
  if (argc < 2) {
    std::cout << "usage: " << argv[0] << " FILE." << std::endl;
    return 0;
  }

  std::string file = argv[1];
  std::cout << "quickfix config file: " << file << std::endl;

  FixServer<common::ServerTraits> server;
  return 0;
}