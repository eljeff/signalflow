#include "signalflow/node/processors/smooth.h"

namespace signalflow
{

Smooth::Smooth(NodeRef input, NodeRef smooth)
    : UnaryOpNode(input), smooth(smooth)
{
    this->name = "smooth";
    this->create_input("smooth", this->smooth);
    this->alloc();
}

void Smooth::alloc()
{
    this->values.resize(this->num_output_channels_allocated);
}

void Smooth::process(Buffer &out, int num_frames)
{
    for (int channel = 0; channel < this->num_output_channels; channel++)
    {
        for (int frame = 0; frame < num_frames; frame++)
        {
            values[channel] = (smooth->out[channel][frame] * values[channel]) + ((1.0 - smooth->out[channel][frame]) * input->out[channel][frame]);
            out[channel][frame] = values[channel];
        }
    }
}

}
