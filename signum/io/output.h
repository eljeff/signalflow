#pragma once

#include <soundio/soundio.h>

#include "../unit.h"
#include "../buffer.h"
#include "../graph.h"

#include <vector>

namespace signum::io
{
	class AudioOut : public Unit
	{
	public:
		AudioOut(Graph *graph);
		int init();
		int start();
		int close();
		virtual sample next();

		Graph *graph;
		struct SoundIo *soundio;
		struct SoundIoDevice *device;
		struct SoundIoOutStream *outstream;
	};
}
