#pragma once

#include "parsers/msgParser.hpp"

/**
 * TransferTransRspParser - parser of transfer transaction response.
 */
class TransferTransRspParser : public MsgParser {
public:
    /**
     * stringify() - turn TransferTransRsp into string.
     *
     * @param msg: TransferTransRsp.
     */
    std::string stringify(std::unique_ptr<Msg>&& msg) override;

    /**
     * parse() - turn string into TransferTransRsp.
     *
     * @param str
     */
    std::unique_ptr<Msg> parse(const std::string& str) override;
};