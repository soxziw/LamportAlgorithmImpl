#pragma once

#include <map>
#include "processors/msgProcessor.hpp"
#include "processors/transferTransReqProcessor.hpp"
#include "processors/transferTransRspProcessor.hpp"
#include "processors/balanceTransReqProcessor.hpp"
#include "processors/balanceTransRspProcessor.hpp"
#include "processors/requestMsgProcessor.hpp"
#include "processors/replyMsgProcessor.hpp"
#include "processors/releaseMsgProcessor.hpp"
#include "processors/finishMsgProcessor.hpp"

/**
 * ProcessorFactory - factory of processors.
 */
class ProcessorFactory {
public:
    /**
     * registerProcessor() - register derived processor class into factory.
     *
     * @param name: registery name.
     * @param creator: function to create derived processor instance.
     */
    void registerProcessor(const std::string& name, std::function<std::unique_ptr<MsgProcessor>()> creator) {
        registry[name] = creator;
    }

    /**
     * createProcessor() - create exist class of processor in factory.
     *
     * @param name: registery name.
     */ 
    std::unique_ptr<MsgProcessor> createProcessor(const std::string& name) {
        if (registry.find(name) != registry.end()) {
            return registry[name]();
        } else {
            std::printf("\033[31m[Error][ProcessorFactory::createProcessor] Unknown processor type: %s.\033[0m\n", name.c_str());
            return nullptr;
        }
    }

private:
    std::map<std::string, std::function<std::unique_ptr<MsgProcessor>()>> registry;
};