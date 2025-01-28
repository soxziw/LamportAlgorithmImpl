#pragma once

#include "parsers/msgParser.hpp"

/**
 * TransferTransReqParser - parser of transfer transaction request.
 */
class TransferTransReqParser : public MsgParser {
public:
    /**
     * stringify() - turn TransferTransReq into string.
     *
     * @param msg: TransferTransReq.
     */
    std::string stringify(std::unique_ptr<Msg>&& msg) override;

    /**
     * parse() - turn string into TransferTransReq.
     *
     * @param str
     */
    std::unique_ptr<Msg> parse(const std::string& str) override;
};