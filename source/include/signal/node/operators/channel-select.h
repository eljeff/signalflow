#pragma once

#include "signal/core/constants.h"
#include "signal/node/node.h"

#include <list>

namespace libsignal
{

class ChannelSelect : public UnaryOpNode
{

public:
    ChannelSelect(NodeRef input = nullptr, int offset = 0, int maximum = 0, int step = 1);

    virtual void process(sample **out, int num_frames);

private:
    int offset;
    int maximum;
    int step;

};

REGISTER(ChannelSelect, "channel-select")

}
