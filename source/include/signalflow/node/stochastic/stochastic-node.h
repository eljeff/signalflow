#pragma once

#include "signalflow/node/node.h"

/*--------------------------------------------------------------------------------
 * This is needed to prevent compile-time errors on Big Sur.
 * vecLib (Accelerate) on macOS 11 provides its own CBLAS defines which
 * clash with the gsl versions. If present, don't also include the gsl ones.
 *-------------------------------------------------------------------------------*/
#ifdef CBLAS_H
#define __GSL_CBLAS_H__
#endif

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <sys/time.h>

#define SIGNALFLOW_PROCESS_STOCHASTIC_NODE_RESET_TRIGGER()             \
    if (SIGNALFLOW_CHECK_CHANNEL_TRIGGER(this->reset, channel, frame)) \
    {                                                                  \
        this->StochasticNode::trigger(SIGNALFLOW_TRIGGER_RESET);       \
    }

namespace signalflow
{

class StochasticNode : public Node
{
public:
    StochasticNode(NodeRef reset = nullptr);
    virtual void trigger(std::string name = SIGNALFLOW_DEFAULT_TRIGGER, float value = 1.0) override;
    virtual void set_seed(unsigned long int seed);

protected:
    double random_uniform(double from, double to);

    NodeRef reset;
    unsigned long int seed;
    gsl_rng *rng;
};

}
