#include "parsers/transferTransRspParser.hpp"
#include "clients/interface.hpp"
#include "clients/lamportServer.hpp"

std::string TransferTransRspParser::stringify(std::unique_ptr<Msg>&& msg) {
    // std::printf("[Server %d] Stringify TransferTransRsp.\n", LamportServer::getInstance()->client_id_);
    TransferTransRsp* msg_raw = dynamic_cast<TransferTransRsp*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("\033[31m[Error][TransferTransRspParser::stringify][Server %d] message does not fit in TransferTransRsp.\033[0m\n", LamportServer::getInstance()->client_id_);
    }
    std::unique_ptr<TransferTransRsp> msg_ptr(static_cast<TransferTransRsp*>(msg.release()));

    // Parse the JSON object into a JSON string
    json msg_json;
    msg_json["type"] = "TransferTransRsp";
    msg_json["transfer_result"] = msg_ptr->transfer_result;
    msg_json["client_id"] = msg_ptr->client_id;
    return msg_json.dump();
}

std::unique_ptr<Msg> TransferTransRspParser::parse(const std::string& str) {
    // std::printf("[Server %d] Parse TransferTransRsp.\n", Interface::getInstance()->client_id_);
    // Parse the JSON string into a JSON object
    json msg_json = json::parse(str);
    return std::make_unique<TransferTransRsp>(msg_json["transfer_result"].get<std::string>(), msg_json["client_id"].get<int>());
}