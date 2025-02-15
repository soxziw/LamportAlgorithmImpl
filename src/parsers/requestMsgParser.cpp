#include "parsers/requestMsgParser.hpp"
#include "clients/lamportServer.hpp"

std::string RequestMsgParser::stringify(std::unique_ptr<Msg>&& msg) {
    // std::printf("[Server %d] Stringify RequestMsg.\n", LamportServer::getInstance()->client_id_);
    RequestMsg* msg_raw = dynamic_cast<RequestMsg*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("\033[31m[Error][RequestMsgParser::stringify][Server %d] message does not fit in RequestMsg.\033[0m\n", LamportServer::getInstance()->client_id_);
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
    // std::printf("[Server %d] Parse RequestMsg.\n", LamportServer::getInstance()->client_id_);
    // Parse the JSON string into a JSON object
    json msg_json = json::parse(str);
    return std::make_unique<RequestMsg>(msg_json["sender_id"].get<int>(), msg_json["receiver_id"].get<int>(), msg_json["amount"].get<int>(), msg_json["lamport_clock"].get<int>(), msg_json["client_id"].get<int>());
}