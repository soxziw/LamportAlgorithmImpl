#pragma once

#include "processors/msgProcessor.hpp"

/**
 * RequestMsgProcessor - processor of request message.
 */
class RequestMsgProcessor : public MsgProcessor {
public:
    /**
     * process()
     *
     * @param msg: RequestMsg.
     */
    int process(std::unique_ptr<Msg>&& msg) override;
};
