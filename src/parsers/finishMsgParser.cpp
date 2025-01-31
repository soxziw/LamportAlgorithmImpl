#include "parsers/finishMsgParser.hpp"
#include "clients/lamportClient.hpp"

std::string FinishMsgParser::stringify(std::unique_ptr<Msg>&& msg) {
    // std::printf("[Client %d] Stringify FinishMsg.\n", LamportClient::getInstance()->client_id_);
    FinishMsg* msg_raw = dynamic_cast<FinishMsg*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("\033[31m[Error][FinishMsgParser::stringify][Client %d] message does not fit in FinishMsg.\033[0m\n", LamportClient::getInstance()->client_id_);
        throw std::bad_cast();
    }
    std::unique_ptr<FinishMsg> msg_ptr(static_cast<FinishMsg*>(msg.release()));

    // Parse the JSON object into a JSON string
    json msg_json;
    msg_json["type"] = "FinishMsg";
    msg_json["lamport_clock"] = msg_ptr->lamport_clock;
    msg_json["client_id"] = msg_ptr->client_id;
    return msg_json.dump();
}

std::unique_ptr<Msg> FinishMsgParser::parse(const std::string& str) {
    // std::printf("[Client %d] Parse FinishMsg.\n", LamportClient::getInstance()->client_id_);
    try {
        // Parse the JSON string into a JSON object
        json msg_json = json::parse(str);
        return std::make_unique<FinishMsg>(msg_json["lamport_clock"].get<int>(), msg_json["client_id"].get<int>());
    } catch (const json::parse_error& e) {
        // Handle parse error and return an empty JSON object
        std::printf("\033[31m[Error][FinishMsgParser::parse][Client %d] Parse error in FinishMsg: %s.\033[0m\n", LamportClient::getInstance()->client_id_, str.c_str());
        return nullptr;
    }
}