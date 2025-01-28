#pragma once

#include "msg.hpp"
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

/**
 * MsgParser - base class of message parsers.
 */
class MsgParser {
public:
    /**
     * ~MsgParser() - default deconstructor.
     *
     * Deconstructor of base class need for deconstruction of derived classes.
     */
    virtual ~MsgParser() = default;

    /**
     * stringify() - turn message structure into string.
     *
     * @param msg
     */
    virtual std::string stringify(std::unique_ptr<Msg>&& msg) = 0;

    /**
     * parse() - turn string into message structure.
     *
     * @param str
     */
    virtual std::unique_ptr<Msg> parse(const std::string& str) = 0;
};
