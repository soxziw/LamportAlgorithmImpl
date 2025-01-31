#include "parsers/replyMsgParser.hpp"
#include "clients/lamportClient.hpp"

std::string ReplyMsgParser::stringify(std::unique_ptr<Msg>&& msg) {
    // std::printf("[Client %d] Stringify ReplyMsg.\n", LamportClient::getInstance()->client_id_);
    ReplyMsg* msg_raw = dynamic_cast<ReplyMsg*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("\033[31m[Error][ReplyMsgParser::stringify][Client %d] message does not fit in ReplyMsg.\033[0m\n", LamportClient::getInstance()->client_id_);
    }
    std::unique_ptr<ReplyMsg> msg_ptr(static_cast<ReplyMsg*>(msg.release()));

    // Parse the JSON object into a JSON string
    json msg_json;
    msg_json["type"] = "ReplyMsg";
    msg_json["lamport_clock"] = msg_ptr->lamport_clock;
    msg_json["client_id"] = msg_ptr->client_id;
    return msg_json.dump();
}

std::unique_ptr<Msg> ReplyMsgParser::parse(const std::string& str) {
    // std::printf("[Client %d] Parse ReplyMsg.\n", LamportClient::getInstance()->client_id_);
    // Parse the JSON string into a JSON object
    json msg_json = json::parse(str);
    return std::make_unique<ReplyMsg>(msg_json["lamport_clock"].get<int>(), msg_json["client_id"].get<int>());
}
