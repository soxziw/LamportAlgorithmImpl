#include "parsers/releaseMsgParser.hpp"
#include "clients/lamportClient.hpp"

std::string ReleaseMsgParser::stringify(std::unique_ptr<Msg>&& msg) {
    // std::printf("[Client %d] Stringify ReleaseMsg.\n", LamportClient::getInstance()->client_id_);
    ReleaseMsg* msg_raw = dynamic_cast<ReleaseMsg*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("\033[31m[Error][ReleaseMsgParser::stringify][Client %d] message does not fit in ReleaseMsg.\033[0m\n", LamportClient::getInstance()->client_id_);
        throw std::bad_cast();
    }
    std::unique_ptr<ReleaseMsg> msg_ptr(static_cast<ReleaseMsg*>(msg.release()));

    // Parse the JSON object into a JSON string
    json msg_json;
    msg_json["type"] = "ReleaseMsg";
    msg_json["lamport_clock"] = msg_ptr->lamport_clock;
    msg_json["client_id"] = msg_ptr->client_id;
    return msg_json.dump();
}

std::unique_ptr<Msg> ReleaseMsgParser::parse(const std::string& str) {
    // std::printf("[Client %d] Parse ReleaseMsg.\n", LamportClient::getInstance()->client_id_);
    try {
        // Parse the JSON string into a JSON object
        json msg_json = json::parse(str);
        return std::make_unique<ReleaseMsg>(msg_json["lamport_clock"].get<int>(), msg_json["client_id"].get<int>());
    } catch (const json::parse_error& e) {
        // Handle parse error and return an empty JSON object
        std::printf("\033[31m[Error][ReleaseMsgParser::parse][Client %d] Parse error in ReleaseMsg: %s.\033[0m\n", LamportClient::getInstance()->client_id_, str.c_str());
        return nullptr;
    }
}

