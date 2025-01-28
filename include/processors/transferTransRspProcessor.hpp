#pragma once

#include "processors/msgProcessor.hpp"

/**
 * BalanceTransReqProcessor - processor of transfer transaction response.
 */
class TransferTransRspProcessor : public MsgProcessor {
public:
    /**
     * process()
     *
     * @param msg: TransferTransRsp.
     */
    int process(std::unique_ptr<Msg>&& msg) override;
};
