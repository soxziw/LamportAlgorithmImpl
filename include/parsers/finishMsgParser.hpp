#pragma once

#include "parsers/msgParser.hpp"

/**
 * FinishMsgParser - parser of finish message.
 */
class FinishMsgParser : public MsgParser {
public:
    /**
     * stringify() - turn FinishMsg into string.
     *
     * @param msg: FinishMsg.
     */
    std::string stringify(std::unique_ptr<Msg>&& msg) override;

    /**
     * parse() - turn string into FinishMsg.
     *
     * @param str
     */
    std::unique_ptr<Msg> parse(const std::string& str) override;
};