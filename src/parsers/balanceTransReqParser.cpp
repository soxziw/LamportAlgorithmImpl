#include "parsers/balanceTransReqParser.hpp"
#include "clients/interfaceClient.hpp"
#include "clients/lamportClient.hpp"


std::string BalanceTransReqParser::stringify(std::unique_ptr<Msg>&& msg) {
    std::printf("[Client %d] Stringify BalanceTransReq.\n", InterfaceClient::getInstance()->client_id_);
    BalanceTransReq* msg_raw = dynamic_cast<BalanceTransReq*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("[ERROR][BalanceTransReqParser::stringify][Client %d] message does not fit in BalanceTransReq.\n", InterfaceClient::getInstance()->client_id_);
        throw std::bad_cast();
    }
    std::unique_ptr<BalanceTransReq> msg_ptr(static_cast<BalanceTransReq*>(msg.release()));

    // Parse the JSON object into a JSON string
    json msg_json;
    msg_json["type"] = "BalanceTransReq";
    msg_json["client_id"] = msg_ptr->client_id;
    return msg_json.dump();
}

std::unique_ptr<Msg> BalanceTransReqParser::parse(const std::string& str) {
    std::printf("[Client %d] Parse BalanceTransReq.\n", LamportClient::getInstance()->client_id_);
    try {
        // Parse the JSON string into a JSON object
        json msg_json = json::parse(str);
        return std::make_unique<BalanceTransReq>(msg_json["client_id"].get<int>());
    } catch (const json::parse_error& e) {
        // Handle parse error and return an empty JSON object
        std::printf("[ERROR][BalanceTransReqParser::parse][Client %d] Parse error in BalanceTransReq: %s.\n", LamportClient::getInstance()->client_id_, str.c_str());
        return nullptr;
    }
}
