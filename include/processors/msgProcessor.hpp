#pragma once
#include "msg.hpp"
#include <memory>

/**
 * MsgProcessor - base class of message processors.
 */
class MsgProcessor {
public:
    /**
     * ~MsgProcessor() - default deconstructor.
     *
     * Deconstructor of base class need for deconstruction of derived classes.
     */
    virtual ~MsgProcessor() = default;

    /**
     * process() - process messages.
     *
     * @param msg
     */
    virtual int process(std::unique_ptr<Msg>&& msg) = 0;
};
