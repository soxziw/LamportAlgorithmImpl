#include "parsers/transferTransRspParser.hpp"
#include "clients/interfaceClient.hpp"
#include "clients/lamportClient.hpp"

std::string TransferTransRspParser::stringify(std::unique_ptr<Msg>&& msg) {
    // std::printf("[Client %d] Stringify TransferTransRsp.\n", LamportClient::getInstance()->client_id_);
    TransferTransRsp* msg_raw = dynamic_cast<TransferTransRsp*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("\033[31m[Error][TransferTransRspParser::stringify][Client %d] message does not fit in TransferTransRsp.\033[0m\n", LamportClient::getInstance()->client_id_);
        throw std::bad_cast();
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
    // std::printf("[Client %d] Parse TransferTransRsp.\n", InterfaceClient::getInstance()->client_id_);
    try {
        // Parse the JSON string into a JSON object
        json msg_json = json::parse(str);
        return std::make_unique<TransferTransRsp>(msg_json["transfer_result"].get<std::string>(), msg_json["client_id"].get<int>());
    } catch (const json::parse_error& e) {
        // Handle parse error and return an empty JSON object
        std::printf("\033[31m[Error][TransferTransRspParser::parse][Client %d] Parse error in TransferTransRsp: %s.\033[0m\n", InterfaceClient::getInstance()->client_id_, str.c_str());
        return nullptr;
    }
}