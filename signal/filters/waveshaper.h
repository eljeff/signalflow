#pragma once

#include "../node.h"
#include "../constants.h"

namespace libsignal
{
    class WaveShaper : public UnaryOpNode
    {
        public:
            WaveShaper(NodeRef input = 0.0, BufferRef buffer = nullptr);

            BufferRef buffer;
            virtual void process(sample **out, int num_frames);
    };

    REGISTER(WaveShaper, "waveshaper");
}
