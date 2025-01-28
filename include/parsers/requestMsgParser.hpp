#pragma once

#include "parsers/msgParser.hpp"

/**
 * RequestMsgParser - parser of request message.
 */
class RequestMsgParser : public MsgParser {
public:
    /**
     * stringify() - turn RequestMsg into string.
     *
     * @param msg: RequestMsg.
     */
    std::string stringify(std::unique_ptr<Msg>&& msg) override;

    /**
     * parse() - turn string into RequestMsg.
     *
     * @param str
     */
    std::unique_ptr<Msg> parse(const std::string& str) override;
};