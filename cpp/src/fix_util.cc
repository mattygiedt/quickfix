#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "quickfix/Message.h"
#include "spdlog/spdlog.h"

static constexpr auto kLineCountNotify = 10000000;
static constexpr auto kFixTimestampLength = 30;
static constexpr auto kDataDictFile = "/workspaces/quickfix/FIX44.xml";

auto main(int argc, char** argv) -> int {
  if (argc < 2) {
    std::cout << "usage: " << argv[0] << " FILE." << std::endl;
    return 1;
  }

  std::int32_t counter{0};

  FIX::DataDictionary data_dictionary{kDataDictFile};

  std::string str;
  std::string file = argv[1];

  std::ifstream infile(file);

  FIX::SettlDate settle_date;
  FIX::QuoteID quote_id;
  FIX::SecurityID security_id;

  FIX::BidPx bid_px;
  FIX::BidSize bid_sz;
  FIX::OfferPx offer_px;
  FIX::OfferSize offer_sz;

  std::vector<std::string> error_vec;
  std::set<FIX::QuoteID> quote_id_set;
  std::set<FIX::QuoteID> non_standard_set;
  std::set<FIX::SecurityID> security_id_set;

  while (std::getline(infile, str)) {
    counter++;

    if (!str.empty()) {
      const auto& msg = str.substr(kFixTimestampLength);
      const auto& fix_msg = FIX::Message(msg);

      if (fix_msg.getFieldIfSet(quote_id) &&
          fix_msg.getFieldIfSet(security_id)) {
        quote_id_set.emplace(quote_id);
        security_id_set.emplace(security_id);

        auto has_settle_date = fix_msg.getFieldIfSet(settle_date);
        if (has_settle_date) {
          if (settle_date.getString() != "20220214") {
            non_standard_set.insert(quote_id);
          }
        }

        if (non_standard_set.count(quote_id) == 1 && !has_settle_date) {
          fix_msg.getField(bid_px);
          fix_msg.getField(bid_sz);
          fix_msg.getField(offer_px);
          fix_msg.getField(offer_sz);

          if (bid_px.getValue() != 0 || bid_sz.getValue() != 0 ||
              offer_px.getValue() != 0 || offer_sz.getValue() != 0) {
            spdlog::warn("invalid quote_id: {}", quote_id.getString());
            // spdlog::warn(msg);
            std::string copy = msg;
            std::replace(copy.begin(), copy.end(), '\001', '|');
            error_vec.emplace_back(copy);
          }
        }
      }
    }

    if (counter % kLineCountNotify == 0) {
      spdlog::info("read {}0M lines ...", (counter / kLineCountNotify));
      spdlog::info(" {} unique security_ids", security_id_set.size());
      spdlog::info(" {} unique quote_ids", quote_id_set.size());
      spdlog::info(" {} unique quote_ids with non-standard settlement",
                   non_standard_set.size());
      spdlog::info(" {} errors", error_vec.size());
    }
  }

  infile.close();

  spdlog::info("read {} total lines", counter);
  spdlog::info(" {} unique security_ids", security_id_set.size());
  spdlog::info(" {} unique quote_ids", quote_id_set.size());
  spdlog::info(" {} unique quote_ids with non-standard settlement",
               non_standard_set.size());
  spdlog::info(" {} errors", error_vec.size());

  for (auto& msg : error_vec) {
    std::cout << msg << std::endl;
  }
  return 0;
}
