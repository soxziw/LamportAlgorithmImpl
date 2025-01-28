#pragma once

#include <map>
#include "parsers/msgParser.hpp"
#include "parsers/transferTransReqParser.hpp"
#include "parsers/transferTransRspParser.hpp"
#include "parsers/balanceTransReqParser.hpp"
#include "parsers/balanceTransRspParser.hpp"
#include "parsers/requestMsgParser.hpp"
#include "parsers/replyMsgParser.hpp"
#include "parsers/releaseMsgParser.hpp"
#include "parsers/finishMsgParser.hpp"

/**
 * ParserFactory - factory of parsers.
 */
class ParserFactory {
public:
    /**
     * registerParser() - register derived parser class into factory.
     *
     * @param name: registery name.
     * @param creator: function to create derived parser instance.
     */
    void registerParser(const std::string& name, std::function<std::unique_ptr<MsgParser>()> creator) {
        registry[name] = creator;
    }
 
    /**
     * createParser() - create exist class of parser in factory.
     *
     * @param name: registery name.
     */ 
    std::unique_ptr<MsgParser> createParser(const std::string& name) {
        if (registry.find(name) != registry.end()) {
            return registry[name]();
        } else {
            std::printf("[ERROR][ParserFactory::createParser] Unknown parser type: %s.\n", name.c_str());
            throw std::runtime_error("[ERROR] Unknown parser type: " + name);
        }
    }

private:
    std::map<std::string, std::function<std::unique_ptr<MsgParser>()>> registry;
};