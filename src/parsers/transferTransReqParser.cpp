#include "parsers/transferTransReqParser.hpp"
#include "clients/interfaceClient.hpp"
#include "clients/lamportClient.hpp"

std::string TransferTransReqParser::stringify(std::unique_ptr<Msg>&& msg) {
    std::printf("[Client %d] Stringify TransferTransReq.\n", InterfaceClient::getInstance()->getClientId());
    TransferTransReq* msg_raw = dynamic_cast<TransferTransReq*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("[ERROR][TransferTransReqParser::stringify][Client %d] message does not fit in TransferTransReq.\n", InterfaceClient::getInstance()->getClientId());
        throw std::bad_cast();
    }
    std::unique_ptr<TransferTransReq> msg_ptr(static_cast<TransferTransReq*>(msg.release()));

    // Parse the JSON object into a JSON string
    json msg_json;
    msg_json["type"] = "TransferTransReq";
    msg_json["sender_id"] = msg_ptr->sender_id;
    msg_json["receiver_id"] = msg_ptr->receiver_id;
    msg_json["amount"] = msg_ptr->amount;
    msg_json["client_id"] = msg_ptr->client_id;
    return msg_json.dump();
}

std::unique_ptr<Msg> TransferTransReqParser::parse(const std::string& str) {
    std::printf("[Client %d] Parse TransferTransReq.\n", LamportClient::getInstance()->getClientId());
    try {
        // Parse the JSON string into a JSON object
        json msg_json = json::parse(str);
        return std::make_unique<TransferTransReq>(msg_json["sender_id"].get<int>(), msg_json["receiver_id"].get<int>(), msg_json["amount"].get<int>(), msg_json["client_id"].get<int>());
    } catch (const json::parse_error& e) {
        // Handle parse error and return an empty JSON object
        std::printf("[ERROR][TransferTransReqParser::parse][Client %d] Parse error in TransferTransReq: %s.\n", LamportClient::getInstance()->getClientId(), str.c_str());
        return nullptr;
    }
}