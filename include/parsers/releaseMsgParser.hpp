#pragma once

#include "parsers/msgParser.hpp"

/**
 * ReleaseMsgParser - parser of release message.
 */
class ReleaseMsgParser : public MsgParser {
public:
    /**
     * stringify() - turn ReleaseMsg into string.
     *
     * @param msg: ReleaseMsg.
     */
    std::string stringify(std::unique_ptr<Msg>&& msg) override;

    /**
     * parse() - turn string into ReleaseMsg.
     *
     * @param str
     */
    std::unique_ptr<Msg> parse(const std::string& str) override;
};
