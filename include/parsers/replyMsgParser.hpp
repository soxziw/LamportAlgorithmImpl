#pragma once

#include "parsers/msgParser.hpp"

/**
 * ReplyMsgParser - parser of reply message.
 */
class ReplyMsgParser : public MsgParser {
public:
    /**
     * stringify() - turn ReplyMsg into string.
     *
     * @param msg: ReplyMsg.
     */
    std::string stringify(std::unique_ptr<Msg>&& msg) override;

    /**
     * parse() - turn string into ReplyMsg.
     *
     * @param str
     */
    std::unique_ptr<Msg> parse(const std::string& str) override;
};