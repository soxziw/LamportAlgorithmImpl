#pragma once

#include "parsers/msgParser.hpp"

/**
 * BalanceTransRspParser - parser of balance transaction response.
 */
class BalanceTransRspParser : public MsgParser {
public:
    /**
     * stringify() - turn BalanceTransRsp into string.
     *
     * @param msg: BalanceTransRsp.
     */
    std::string stringify(std::unique_ptr<Msg>&& msg) override;

    /**
     * parse() - turn string into BalanceTransRsp.
     *
     * @param str
     */
    std::unique_ptr<Msg> parse(const std::string& str) override;
};
