#include "signal/synth/synthspec.h"

#include "signal/core/core.h"
#include "signal/node/registry.h"
#include "signal/synth/synthregistry.h"
#include "signal/node/oscillators/constant.h"
#include "json11/json11.hpp"

#include <iostream>
#include <fstream>

using namespace json11;

namespace libsignal
{
    SynthSpec::SynthSpec(std::string name)
    {
        this->name = name;
    }
    
    void SynthSpec::save(std::string filename)
    {
    }

    void SynthSpec::load(std::string filename)
    {
        std::string buf;
        std::string line;
        std::string err;

        std::ifstream input(filename);
        while (std::getline(input, line))
        {
            buf += line + "\n";
        }

        auto json = Json::parse(buf, err);
        if (!err.empty())
        {
            signal_warn("Failed: %s\n", err.c_str());
        }

        if (!json.is_array())
        {
            signal_warn("Cannot parse JSON (root element must be array)\n");
            return;
        }
        for (auto element : json.array_items())
        {
            NodeDefinition node;
            bool is_output = false;
            for (auto pair : element.object_items())
            {
                std::string key = pair.first;
                auto value = pair.second;

                if (key == "node")
                {
                    node.set_name(value.string_value());
                }
                else if (key == "id")
                {
                    node.set_id(value.int_value());
                }
                else if (key == "is_output")
                {
                    is_output = true;
                }
                else if (key == "inputs")
                {
                    for (auto input_pair : value.object_items())
                    {
                        std::string input_key = input_pair.first;
                        auto input_value = input_pair.second;

                        if (input_value.is_number())
                        {
                            node.add_input(input_key, input_value.number_value());
                        }
                        else if (input_value.is_object())
                        {
                            int id = input_value["id"].int_value();
                            NodeDefinition *ptr = this->get_node_def(id);
                            node.add_input(input_key, ptr);
                        }
                    }
                }
            }
            signal_debug("Adding node with name %s\n", node.name.c_str());
            this->add_node_def(node);
            if (is_output)
            {
                this->set_output(node);
            }
        }
        
        /*------------------------------------------------------------------------
         * Set `parsed` to indicate we have a complete NodeDef tree
         * (normally used to indicate the completion of template-based
         * construction)
         *-----------------------------------------------------------------------*/
        this->parsed = true;
    }

    void SynthSpec::store()
    {
        SynthRegistry::global()->add(this->name, this);
    }

    void SynthSpec::add_node_def(NodeDefinition def)
    {
        this->nodedefs[def.id] = def;
    } 

    NodeDefinition *SynthSpec::get_node_def(int id)
    {
        return &(this->nodedefs[id]);
    } 

    void SynthSpec::set_output(NodeDefinition def)
    {
        this->output_def = def;
    } 


    NodeDefinition SynthSpec::get_root()
    {
        return this->output_def;
    }

}