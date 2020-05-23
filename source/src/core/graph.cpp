#include "signal/core/graph.h"
#include "signal/core/core.h"
#include "signal/core/graph-monitor.h"
#include "signal/node/node.h"
#include "signal/node/oscillators/constant.h"

#include "signal/patch/patch.h"

#include "signal/node/io/output/abstract.h"
#include "signal/node/io/output/ios.h"
#include "signal/node/io/output/soundio.h"

#include <unistd.h>
#include <sys/time.h>

namespace libsignal
{

AudioGraph *shared_graph = NULL;

AudioGraph::AudioGraph()
{
    signal_init();

    if (shared_graph)
    {
        throw std::runtime_error("Graph already instantiated");
    }
    shared_graph = this;

    AudioOut *audioout = new AudioOut(this);
    this->output = audioout;
    this->sample_rate = audioout->sample_rate;
    this->node_count = 0;
    this->cpu_usage = 0.0;
    this->monitor = NULL;
}

void AudioGraph::start()
{
    AudioOut *audioout = (AudioOut *) this->output.get();
    audioout->start();
}

AudioGraph::~AudioGraph()
{
    AudioOut *audioout = (AudioOut *) this->output.get();
    audioout->destroy();
    shared_graph = NULL;
}

void AudioGraph::wait(float time)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double t0 = tv.tv_sec + tv.tv_usec / 1000000.0;

    while (true)
    {
        usleep(10000);
        if (time)
        {
            gettimeofday(&tv, NULL);
            double t1 = tv.tv_sec + tv.tv_usec / 1000000.0;
            double dt = t1 - t0;
            if (dt > time)
            {
                break;
            }
        }
    }
}

void AudioGraph::pull_input(const NodeRef &node, int num_frames)
{
    /*------------------------------------------------------------------------
     * If this node has already been processed this timestep, return.
     * TODO: This will become increasingly costly as the # nodes increases.
     *       Timestamp and consider adding a flag in Node?
     *-----------------------------------------------------------------------*/
    if (this->processed_nodes.find(node.get()) != processed_nodes.end())
    {
        signal_debug("Already processed node %s, skipping", node->name.c_str());
        return;
    }

    if (!(node->inputs.size() > 0 || node->name == "constant" || node->name == "audioout" || node->name == "audioin"))
    {
        signal_debug("Node %s has no registered inputs", node->name.c_str());
    }

    /*------------------------------------------------------------------------
     * Pull our inputs before we generate our own outputs.
     *-----------------------------------------------------------------------*/
    for (auto param : node->inputs)
    {
        NodeRef param_node = *(param.second);
        if (param_node)
        {
            this->pull_input(param_node, num_frames);

            /*------------------------------------------------------------------------
             * Automatic input upmix.
             *
             * If the input node produces less channels than demanded, automatically
             * up-mix its output by replicating the existing channels. This allows
             * operations between multi-channel and mono-channel inputs to work
             * seamlessly without any additional implementation within the node
             * itself (for example, Multiply(new Sine(440), new LinearPanner(2, ...)))
             *
             * A few nodes must prevent automatic input up-mixing from happening.
             * These include ChannelArray and AudioOut.
             *-----------------------------------------------------------------------*/
            if (param_node->num_output_channels < node->num_input_channels && !node->no_input_upmix)
            {
                signal_debug("Upmixing %s (%s wants %d channels, %s only produces %d)", param_node->name.c_str(),
                             node->name.c_str(), node->num_input_channels, param_node->name.c_str(), param_node->num_output_channels);

                /*------------------------------------------------------------------------
                 * If we generate 2 channels but have 6 channels demanded, repeat
                 * them: [ 0, 1, 0, 1, 0, 1 ]
                 *-----------------------------------------------------------------------*/
                for (int out_channel_index = param_node->num_output_channels;
                     out_channel_index < node->num_input_channels;
                     out_channel_index++)
                {
                    int in_channel_index = out_channel_index % param_node->num_output_channels;
                    memcpy(param_node->out[out_channel_index],
                           param_node->out[in_channel_index],
                           num_frames * sizeof(sample));
                }
            }
        }
    }

    node->_process(node->out, num_frames);

    if (node->name != "constant")
        this->processed_nodes.insert(node.get());
}

void AudioGraph::pull_input(int num_frames)
{
    /*------------------------------------------------------------------------
     * Timestamp the start of processing to measure CPU usage.
     *-----------------------------------------------------------------------*/
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double t0 = tv.tv_sec + tv.tv_usec / 1000000.0;

    AudioOut_Abstract *output = (AudioOut_Abstract *) this->output.get();

    /*------------------------------------------------------------------------
     * Disconnect any nodes and patchs that are scheduled to be removed.
     *-----------------------------------------------------------------------*/
    for (auto node : nodes_to_remove)
    {
        output->remove_input(node);
    }
    nodes_to_remove.clear();

    for (auto patch : patchs_to_remove)
    {
        for (auto patchref : patchs)
        {
            if (patchref.get() == patch)
            {
                patchs.erase(patchref);
                break;
            }
        }
    }
    this->patchs_to_remove.clear();

    /*------------------------------------------------------------------------
     * Clear the record of processed nodes, and begin recursing through the
     * node tree to process each node in turn.
     *-----------------------------------------------------------------------*/
    this->processed_nodes.clear();
    this->pull_input(this->output, num_frames);
    this->node_count = this->processed_nodes.size();
    signal_debug("AudioGraph: pull %d frames, %d nodes", num_frames, this->node_count);

    /*------------------------------------------------------------------------
     * Calculate CPU usage (approximately) by measuring the % of time
     * within the audio I/O callback that was used for processing.
     *-----------------------------------------------------------------------*/
    gettimeofday(&tv, NULL);
    double t1 = tv.tv_sec + tv.tv_usec / 1000000.0;
    double dt = t1 - t0;
    double t_max = (double) num_frames / this->sample_rate;
    this->cpu_usage = dt / t_max;
}

void AudioGraph::process(const NodeRef &root, int num_frames, int block_size)
{
    /*------------------------------------------------------------------------
     * Updating the routing from the tip. 
     * This normally happens when the root is connected to the graph's
     * audio out. Can this be improved?
     *-----------------------------------------------------------------------*/
    root->update_channels();

    int index = 0;
    signal_debug("AudioGraph: Performing offline process of %d frames", num_frames);
    while (index < (num_frames - block_size))
    {
        signal_debug("AudioGraph: Processing frame %d...", index);
        this->processed_nodes.clear();
        this->pull_input(root, block_size);
        index += block_size;
    }

    /*------------------------------------------------------------------------
     * Process remaining samples.
     *-----------------------------------------------------------------------*/
    if (index < num_frames)
    {
        signal_debug("AudioGraph: Processing remaining %d samples", num_frames - index);
        this->processed_nodes.clear();
        this->pull_input(root, num_frames - index);
    }

    signal_debug("AudioGraph: Offline process completed");
}

NodeRef AudioGraph::get_output()
{
    return this->output;
}

void AudioGraph::add_output(PatchRef patch)
{
    AudioOut_Abstract *output = (AudioOut_Abstract *) (this->output.get());
    output->add_input(patch->output);
    this->patchs.insert(patch);
}

void AudioGraph::add_output(NodeRef node)
{
    AudioOut_Abstract *output = (AudioOut_Abstract *) this->output.get();
    output->add_input(node);
}

void AudioGraph::remove_output(PatchRef patch)
{
    this->remove_output(patch.get());
}

void AudioGraph::remove_output(Patch *patch)
{
    patchs_to_remove.insert(patch);
    nodes_to_remove.insert(patch->output);
}

void AudioGraph::remove_output(NodeRef node)
{
    nodes_to_remove.insert(node);
}

void AudioGraph::print()
{
    std::cout << "AudioGraph" << std::endl;
    this->print(this->output, 0);
}

void AudioGraph::print(NodeRef &root, int depth)
{
    std::cout << std::string(depth * 2, ' ');
    std::cout << " * " << root->name << std::endl;
    for (auto pair : root->inputs)
    {
        std::cout << std::string((depth + 1) * 2 + 1, ' ');

        NodeRef param_node = *(pair.second);
        if (param_node->name == "constant")
        {
            Constant *constant = (Constant *) (param_node.get());
            std::cout << pair.first << ": " << constant->value << std::endl;
        }
        else
        {
            std::cout << pair.first << ":" << std::endl;
            this->print(param_node, depth + 1);
        }
    }
}

void AudioGraph::poll(float frequency)
{
    if (frequency > 0)
    {
        if (this->monitor)
        {
            throw std::runtime_error("AudioGraph is already polling state");
        }
        this->monitor = new AudioGraphMonitor(this, frequency);
        this->monitor->start();
    }
}

int AudioGraph::get_sample_rate()
{
    return this->sample_rate;
}

void AudioGraph::set_sample_rate(int sample_rate)
{
    if (sample_rate <= 0)
    {
        throw std::runtime_error("Sample rate cannot be <= 0");
    }
    this->sample_rate = sample_rate;
}

int AudioGraph::get_node_count()
{
    return this->node_count;
}

int AudioGraph::get_patch_count()
{
    return (int) this->patchs.size();
}

float AudioGraph::get_cpu_usage()
{
    return this->cpu_usage;
}


}
