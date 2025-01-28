#pragma once

#include "processors/msgProcessor.hpp"

/**
 * BalanceTransRspProcessor - processor of balance transaction response.
 */
class BalanceTransRspProcessor : public MsgProcessor {
public:
    /**
     * process()
     *
     * @param msg: BalanceTransRsp.
     */
    int process(std::unique_ptr<Msg>&& msg) override;
};
