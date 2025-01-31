#pragma once

#include "parsers/msgParser.hpp"

/**
 * ExitMsgParser - parser of exit message.
 */
class ExitMsgParser : public MsgParser {
public:
    /**
     * stringify() - turn ExitMsg into string.
     *
     * @param msg: ExitMsg.
     */
    std::string stringify(std::unique_ptr<Msg>&& msg) override;

    /**
     * parse() - turn string into ExitMsg.
     *
     * @param str
     */
    std::unique_ptr<Msg> parse(const std::string& str) override;
};