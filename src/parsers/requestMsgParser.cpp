#include "parsers/requestMsgParser.hpp"
#include "clients/lamportClient.hpp"

std::string RequestMsgParser::stringify(std::unique_ptr<Msg>&& msg) {
    std::printf("[Client %d] Stringify RequestMsg.\n", LamportClient::getInstance()->getClientId());
    RequestMsg* msg_raw = dynamic_cast<RequestMsg*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("[ERROR][RequestMsgParser::stringify][Client %d] message does not fit in RequestMsg.\n", LamportClient::getInstance()->getClientId());
        throw std::bad_cast();
    }
    std::unique_ptr<RequestMsg> msg_ptr(static_cast<RequestMsg*>(msg.release()));

    // Parse the JSON object into a JSON string
    json msg_json;
    msg_json["type"] = "RequestMsg";
    msg_json["sender_id"] = msg_ptr->sender_id;
    msg_json["receiver_id"] = msg_ptr->receiver_id;
    msg_json["amount"] = msg_ptr->amount;
    msg_json["lamport_clock"] = msg_ptr->lamport_clock;
    msg_json["client_id"] = msg_ptr->client_id;
    return msg_json.dump();
}

std::unique_ptr<Msg> RequestMsgParser::parse(const std::string& str) {
    std::printf("[Client %d] Parse RequestMsg.\n", LamportClient::getInstance()->getClientId());
    try {
        // Parse the JSON string into a JSON object
        json msg_json = json::parse(str);
        return std::make_unique<RequestMsg>(msg_json["sender_id"].get<int>(), msg_json["receiver_id"].get<int>(), msg_json["amount"].get<int>(), msg_json["lamport_clock"].get<int>(), msg_json["client_id"].get<int>());
    } catch (const json::parse_error& e) {
        // Handle parse error and return an empty JSON object
        std::printf("[ERROR][RequestMsgParser::parse][Client %d] Parse error in RequestMsg: %s.\n", LamportClient::getInstance()->getClientId(), str.c_str());
        return nullptr;
    }
}