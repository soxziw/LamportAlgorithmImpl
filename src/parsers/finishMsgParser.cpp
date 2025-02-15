#include "parsers/finishMsgParser.hpp"
#include "clients/lamportServer.hpp"

std::string FinishMsgParser::stringify(std::unique_ptr<Msg>&& msg) {
    // std::printf("[Server %d] Stringify FinishMsg.\n", LamportServer::getInstance()->client_id_);
    FinishMsg* msg_raw = dynamic_cast<FinishMsg*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("\033[31m[Error][FinishMsgParser::stringify][Server %d] message does not fit in FinishMsg.\033[0m\n", LamportServer::getInstance()->client_id_);
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
    // std::printf("[Server %d] Parse FinishMsg.\n", LamportServer::getInstance()->client_id_);
    // Parse the JSON string into a JSON object
    json msg_json = json::parse(str);
    return std::make_unique<FinishMsg>(msg_json["lamport_clock"].get<int>(), msg_json["client_id"].get<int>());
}