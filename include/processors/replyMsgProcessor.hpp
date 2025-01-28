#pragma once

#include "processors/msgProcessor.hpp"

/**
 * ReplyMsgProcessor - processor of reply message.
 */
class ReplyMsgProcessor : public MsgProcessor {
public:
    /**
     * process()
     *
     * @param msg: ReplyMsg.
     */
    int process(std::unique_ptr<Msg>&& msg) override;
};
