#pragma once

#include "processors/msgProcessor.hpp"

/**
 * FinishMsgProcessor - processor of finish message.
 */
class FinishMsgProcessor : public MsgProcessor {
public:
    /**
     * process()
     *
     * @param msg: FinishMsg.
     */
    int process(std::unique_ptr<Msg>&& msg) override;
};
