#include "parsers/balanceTransReqParser.hpp"
#include "clients/interface.hpp"
#include "clients/lamportServer.hpp"


std::string BalanceTransReqParser::stringify(std::unique_ptr<Msg>&& msg) {
    // std::printf("[Server %d] Stringify BalanceTransReq.\n", Interface::getInstance()->client_id_);
    BalanceTransReq* msg_raw = dynamic_cast<BalanceTransReq*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("\033[31m[Error][BalanceTransReqParser::stringify][Server %d] message does not fit in BalanceTransReq.\033[0m\n", Interface::getInstance()->client_id_);
    }
    std::unique_ptr<BalanceTransReq> msg_ptr(static_cast<BalanceTransReq*>(msg.release()));

    // Parse the JSON object into a JSON string
    json msg_json;
    msg_json["type"] = "BalanceTransReq";
    msg_json["client_id"] = msg_ptr->client_id;
    return msg_json.dump();
}

std::unique_ptr<Msg> BalanceTransReqParser::parse(const std::string& str) {
    // std::printf("[Server %d] Parse BalanceTransReq.\n", LamportServer::getInstance()->client_id_);
    // Parse the JSON string into a JSON object
    json msg_json = json::parse(str);
    return std::make_unique<BalanceTransReq>(msg_json["client_id"].get<int>());
}
