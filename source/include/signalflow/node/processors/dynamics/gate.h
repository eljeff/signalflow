#pragma once

#include "signalflow/core/constants.h"
#include "signalflow/node/node.h"

namespace signalflow
{
class Gate : public UnaryOpNode
{
public:
    Gate(NodeRef input = 0.0, NodeRef threshold = 0.1);

    virtual void process(Buffer &out, int num_frames);

    NodeRef threshold;

private:
    float level;
};

REGISTER(Gate, "gate")
}
