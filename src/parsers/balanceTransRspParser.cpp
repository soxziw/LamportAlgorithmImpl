#include "parsers/balanceTransRspParser.hpp"
#include "clients/interface.hpp"
#include "clients/lamportServer.hpp"

std::string BalanceTransRspParser::stringify(std::unique_ptr<Msg>&& msg) {
    // std::printf("[Server %d] Stringify BalanceTransRsp.\n", LamportServer::getInstance()->client_id_);
    BalanceTransRsp* msg_raw = dynamic_cast<BalanceTransRsp*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("\033[31m[Error][BalanceTransRspParser::stringify][Server %d] message does not fit in BalanceTransRsp.\033[0m\n", LamportServer::getInstance()->client_id_);
    }
    std::unique_ptr<BalanceTransRsp> msg_ptr(static_cast<BalanceTransRsp*>(msg.release()));

    // Parse the JSON object into a JSON string
    json msg_json;
    msg_json["type"] = "BalanceTransRsp";
    json client_balance_pairs_json;
    for (const std::pair<int, int>& pair : msg_ptr->client_balance_pairs) {
        client_balance_pairs_json.push_back({{"client_id", pair.first}, {"balance", pair.second}});
    }
    msg_json["client_balance_pairs"] = client_balance_pairs_json;
    msg_json["client_id"] = msg_ptr->client_id;
    return msg_json.dump();
}

std::unique_ptr<Msg> BalanceTransRspParser::parse(const std::string& str) {
    // std::printf("[Server %d] Parse BalanceTransRsp.\n", Interface::getInstance()->client_id_);
    // Parse the JSON string into a JSON object
    json msg_json = json::parse(str);
    std::vector<std::pair<int, int>> client_balance_pairs;
    for (const auto& pair : msg_json["client_balance_pairs"]) {
        client_balance_pairs.push_back(std::make_pair(pair["client_id"].get<int>(), pair["balance"].get<int>()));
    }
    return std::make_unique<BalanceTransRsp>(client_balance_pairs, msg_json["client_id"].get<int>());
}
