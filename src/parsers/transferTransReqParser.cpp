#include "parsers/transferTransReqParser.hpp"
#include "clients/interfaceClient.hpp"
#include "clients/lamportClient.hpp"

std::string TransferTransReqParser::stringify(std::unique_ptr<Msg>&& msg) {
    // std::printf("[Client %d] Stringify TransferTransReq.\n", InterfaceClient::getInstance()->client_id_);
    TransferTransReq* msg_raw = dynamic_cast<TransferTransReq*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("\033[31m[Error][TransferTransReqParser::stringify][Client %d] message does not fit in TransferTransReq.\033[0m\n", InterfaceClient::getInstance()->client_id_);
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
    // std::printf("[Client %d] Parse TransferTransReq.\n", LamportClient::getInstance()->client_id_);
    // Parse the JSON string into a JSON object
    json msg_json = json::parse(str);
    return std::make_unique<TransferTransReq>(msg_json["sender_id"].get<int>(), msg_json["receiver_id"].get<int>(), msg_json["amount"].get<int>(), msg_json["client_id"].get<int>());
}