#pragma once

#include "processors/msgProcessor.hpp"

/**
 * BalanceTransReqProcessor - processor of balance transaction request.
 */
class BalanceTransReqProcessor : public MsgProcessor {
public:
    /**
     * process()
     *
     * @param msg: BalanceTransReq.
     */
    int process(std::unique_ptr<Msg>&& msg) override;
};