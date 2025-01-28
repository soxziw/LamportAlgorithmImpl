#pragma once

#include "processors/msgProcessor.hpp"

/**
 * ReleaseMsgProcessor - processor of release message.
 */
class ReleaseMsgProcessor : public MsgProcessor {
public:
    /**
     * process()
     *
     * @param msg: ReleaseMsg.
     */
    int process(std::unique_ptr<Msg>&& msg) override;
};
