#pragma once

#include "parsers/msgParser.hpp"

/**
 * BalanceTransReqParser - parser of balance transaction request.
 */
class BalanceTransReqParser : public MsgParser {
public:
    /**
     * stringify() - turn BalanceTransReq into string.
     *
     * @param msg: BalanceTransReq.
     */
    std::string stringify(std::unique_ptr<Msg>&& msg) override;

    /**
     * parse() - turn string into BalanceTransReq.
     *
     * @param str
     */
    std::unique_ptr<Msg> parse(const std::string& str) override;
};