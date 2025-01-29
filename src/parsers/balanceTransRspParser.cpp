#include "parsers/balanceTransRspParser.hpp"
#include "clients/interfaceClient.hpp"
#include "clients/lamportClient.hpp"

std::string BalanceTransRspParser::stringify(std::unique_ptr<Msg>&& msg) {
    std::printf("[Client %d] Stringify BalanceTransRsp.\n", LamportClient::getInstance()->client_id_);
    BalanceTransRsp* msg_raw = dynamic_cast<BalanceTransRsp*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("[ERROR][BalanceTransRspParser::stringify][Client %d] message does not fit in BalanceTransRsp.\n", LamportClient::getInstance()->client_id_);
        throw std::bad_cast();
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
    std::printf("[Client %d] Parse BalanceTransRsp.\n", InterfaceClient::getInstance()->client_id_);
    try {
        // Parse the JSON string into a JSON object
        json msg_json = json::parse(str);
        std::vector<std::pair<int, int>> client_balance_pairs;
        for (const auto& pair : msg_json["client_balance_pairs"]) {
            client_balance_pairs.push_back(std::make_pair(pair["client_id"].get<int>(), pair["balance"].get<int>()));
        }
        return std::make_unique<BalanceTransRsp>(client_balance_pairs, msg_json["client_id"].get<int>());
    } catch (const json::parse_error& e) {
        // Handle parse error and return an empty JSON object
        std::printf("[ERROR][BalanceTransRspParser::parse][Client %d] Parse error in BalanceTransRsp: %s.\n", InterfaceClient::getInstance()->client_id_, str.c_str());
        return nullptr;
    }
}
