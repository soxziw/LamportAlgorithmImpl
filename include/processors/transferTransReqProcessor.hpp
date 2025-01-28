#pragma once

#include "processors/msgProcessor.hpp"

/**
 * BalanceTransReqProcessor - processor of transfer transaction request.
 */
class TransferTransReqProcessor : public MsgProcessor {
public:
    /**
     * process()
     *
     * @param msg: TransferTransReq.
     */
    int process(std::unique_ptr<Msg>&& msg) override;
};
