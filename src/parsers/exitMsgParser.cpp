#include "parsers/exitMsgParser.hpp"
#include "clients/interface.hpp"

std::string ExitMsgParser::stringify(std::unique_ptr<Msg>&& msg) {
    // std::printf("[Server %d] Stringify ExitMsg.\n", Interface::getInstance()->client_id_);
    ExitMsg* msg_raw = dynamic_cast<ExitMsg*>(msg.get());
    if (!msg_raw) { // Could not cast
        std::printf("\033[31m[Error][ExitMsgParser::stringify][Server %d] message does not fit in ExitMsg.\033[0m\n", Interface::getInstance()->client_id_);
    }
    std::unique_ptr<ExitMsg> msg_ptr(static_cast<ExitMsg*>(msg.release()));

    // Parse the JSON object into a JSON string
    json msg_json;
    msg_json["type"] = "ExitMsg";
    return msg_json.dump();
}

std::unique_ptr<Msg> ExitMsgParser::parse(const std::string& str) {
    // std::printf("[Server %d] Parse ExitMsg.\n", Interface::getInstance()->client_id_);
    // Parse the JSON string into a JSON object
    json msg_json = json::parse(str);
    return std::make_unique<ExitMsg>();
}